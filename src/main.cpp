//
// Created by wdian on 2021/7/1.
//
#include <cstdio>
#include <cstring>
#include <istream>
#include <fstream>
#include <iostream>
#include "../include/calibrate.h"

using namespace std;

int main() {
//    void calibrate(const float *hist_ptr,
//                   const int hist_size,
//                   const float *hist_edges_ptr,
//                   float * out_threshold,
//                   float * out_divergence,
//                   int num_quantized_bins);

    std::string f1 = "/opt3/A_Projects/calibrate/test/data/hist.bin";
    std::string f2 ="/opt3/A_Projects/calibrate/test/data/hist_edges.bin";
    char *hist;
    char *hist_edges;
    int size;

    ifstream ifs1(f1, ios::in |ios_base::binary);
    if(ifs1){
        ifs1.seekg( 0, ios::end );
        size = ifs1.tellg();
        ifs1.seekg( 0, ios::beg);
        hist = new char[size];
        cout<<size<<endl;
        ifs1.read(hist, size);

    }

    ifstream ifs2(f2, ios::in |ios_base::binary);

    if(ifs2){
        ifs2.seekg( 0, ios::end );
        size = ifs2.tellg();
        hist_edges = new char[size];
        ifs2.seekg(0,ios::beg);
        cout<<size<<endl;
        ifs2.read(hist_edges, size);

    }
    ifs1.close();
    ifs2.close();

    auto *out_threshold =new float[1];
    auto *out_divergence = new float[1];

    calibrate(reinterpret_cast<float *>(hist), size/4, reinterpret_cast<float *>(hist_edges), out_threshold, out_divergence, 255);

    cout<<*out_threshold<<endl;
    cout<<*out_divergence<<endl;

}
