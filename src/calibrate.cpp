#include "../include/calibrate.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
//#include <thread>
#include <omp.h>

int get_cpu_processors() {
    //uint8_t thread_num = std::thread::hardware_concurrency();
    uint8_t thread_num = omp_get_num_procs();
    return thread_num;
}

std::vector<float> SmoothDistribution(const std::vector<float>& p, const float eps = 0.0001) {
    std::vector<size_t> is_zeros(p.size());
    std::vector<size_t> is_nonzeros(p.size());
    {
        auto it = p.begin();
        std::generate(is_zeros.begin(), is_zeros.end(),
                      [&it]() { return static_cast<size_t>(*(it++) == 0.f); });
    }
    {
        auto it = p.begin();
        std::generate(is_nonzeros.begin(), is_nonzeros.end(),
                      [&it]() { return static_cast<size_t>(*(it++) != 0.f); });
    }

    size_t n_zeros = std::accumulate(is_zeros.begin(), is_zeros.end(), 0);
    size_t n_nonzeros = p.size() - n_zeros;
    if (!n_nonzeros) {
        // The discrete probability distribution is malformed. All entries are 0.
        return std::vector<float>();
    }
    float eps1 = eps * static_cast<float>(n_zeros) / static_cast<float>(n_nonzeros);
    if (eps1 >= 1.0) return std::vector<float>();
    auto ret = p;
    for (size_t i = 0; i < p.size(); i++) {
        ret[i] += eps * is_zeros[i] - eps1 * is_nonzeros[i];
    }
    return ret;
}

static float ComputeEntropy(std::vector<float>* p_ptr, std::vector<float>* q_ptr) {
    std::vector<float>& p = *p_ptr;
    std::vector<float>& q = *q_ptr;
    // CHECK_EQ(p.size(), q.size());
    float p_sum = std::accumulate(p.begin(), p.end(), 0.f);
    float q_sum = std::accumulate(q.begin(), q.end(), 0.f);
    for (auto& it : p) {
        it = it / p_sum;
    }

    for (auto& it : q) {
        it = it / q_sum;
    }
    float ret = 0;
    for (size_t i = 0; i < p.size(); i++) {
        // CHECK(p[i] > 0 && q[i] > 0);
        if (p[i] && q[i]) ret += p[i] * std::log(p[i] / q[i]);
    }
    return ret;
}

void calibrate(const float* hist_ptr,
               const int hist_size,
               const float* hist_edges_ptr,
               float* out_threshold,
               float* out_divergence,
               int num_quantized_bins) {

    const int num_bins = hist_size;
    const int zero_bin_idx = num_bins / 2;
    const int num_half_quantized_bins = num_quantized_bins / 2;
    std::vector<float> thresholds(num_bins / 2 + 1 - num_quantized_bins / 2, 0.f);
    std::vector<float> divergence(thresholds.size(), 0.f);

    #pragma omp parallel for num_threads(get_cpu_processors() - 1)
    for (int i = num_quantized_bins / 2; i <= zero_bin_idx; i++) {
        const size_t p_bin_idx_start = zero_bin_idx - i;
        const size_t p_bin_idx_stop = zero_bin_idx + i + 1;
        thresholds[i - num_half_quantized_bins] = hist_edges_ptr[p_bin_idx_stop];

        std::vector<size_t> sliced_nd_hist(p_bin_idx_stop - p_bin_idx_start);
        std::vector<float> p(p_bin_idx_stop - p_bin_idx_start);
        p[0] = 0;
        p.back() = 0;
        for (size_t j = 0; j < num_bins; j++) {
            //std::cout<<hist_ptr[j]<<std::endl;
            if (j <= p_bin_idx_start) {
                p[0] += hist_ptr[j];
            } else if (j >= p_bin_idx_stop) {
                p.back() += hist_ptr[j];
            } else {
                sliced_nd_hist[j - p_bin_idx_start] = hist_ptr[j];
                p[j - p_bin_idx_start] = hist_ptr[j];
            }
        }
        // calculate how many bins should be merged to generate quantized distribution q
        const auto num_merged_bins = sliced_nd_hist.size() / num_quantized_bins;
        // merge hist into num_quantized_bins bins
        std::vector<float> quantized_bins(num_quantized_bins, 0);
        for (int j = 0; j < num_quantized_bins; j++) {
            const int start = j * num_merged_bins;
            const int stop = (j + 1) * num_merged_bins;
            quantized_bins[j] =
                    std::accumulate(sliced_nd_hist.begin() + start, sliced_nd_hist.begin() + stop, 0);
        }
        quantized_bins.back() += std::accumulate(
                sliced_nd_hist.begin() + static_cast<int>(num_quantized_bins * num_merged_bins),
                sliced_nd_hist.end(), 0);
        // expand quantized_bins into p.size bins
        std::vector<float> q(sliced_nd_hist.size(), 0);
        for (int j = 0; j < num_quantized_bins; j++) {
            const int start = j * num_merged_bins;
            const int stop = (j == num_quantized_bins - 1) ? q.size() : ((j + 1) * num_merged_bins);
            int norm = std::count_if(sliced_nd_hist.begin() + start, sliced_nd_hist.begin() + stop,
                                     [](size_t i) { return i != 0; });
            if (norm) {
                for (int k = start; k < stop; k++) {
                    if (p[k]) q[k] = quantized_bins[j] / norm;
                }
            }
        }
        p = SmoothDistribution(p);
        q = SmoothDistribution(q);

        if (!q.size()) {
            divergence[i - num_half_quantized_bins] = std::numeric_limits<float>::infinity();
        } else {
            divergence[i - num_half_quantized_bins] = ComputeEntropy(&p, &q);
        }
    }

    size_t min_divergence_idx = 0;
    float min_divergence = divergence[0];
    for (size_t i = 0; i < divergence.size(); i++) {
        if (divergence[i] < min_divergence) {
            min_divergence = divergence[i];
            min_divergence_idx = i;
        }
    }
    *out_divergence = min_divergence;
    *out_threshold = thresholds[min_divergence_idx];
    // std::cout<<*out_threshold<<std::endl;
    // std::cout<<*out_divergence<<std::endl;
}