#ifndef KL_SO_CALIBRATE_H
#define KL_SO_CALIBRATE_H
extern "C" {
void calibrate(const float *hist_ptr,
               const int hist_size,
               const float *hist_edges_ptr,
               float *out_threshold,
               float *out_divergence,
               int num_quantized_bins);
};

#endif //KL_SO_CALIBRATE_H