// Copyright 2014 BVLC and contributors.

#include <algorithm>
#include <cfloat>
#include <queue>
#include <vector>

#include "caffe/layer.hpp"
#include "caffe/vision_layers.hpp"

namespace caffe {

template <typename Dtype>
void ArgMaxLayer<Dtype>::SetUp(const vector<Blob<Dtype>*>& bottom,
      vector<Blob<Dtype>*>* top) {
  Layer<Dtype>::SetUp(bottom, top);
  out_max_val_ = this->layer_param_.argmax_param().out_max_val();
  top_k_ = this->layer_param_.argmax_param().top_k();
  CHECK_GE(top_k_, 1) << " top k must not be less than 1.";
  CHECK_LE(top_k_, bottom[0]->count() / bottom[0]->num())
      << "top_k must be less than or equal to the number of classes.";
  if (out_max_val_) {
    // Produces max_ind and max_val
    (*top)[0]->Reshape(bottom[0]->num(), 2, top_k_, 1);
  } else {
    // Produces only max_ind
    (*top)[0]->Reshape(bottom[0]->num(), 1, top_k_, 1);
  }
}

template <typename Dtype>
Dtype ArgMaxLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    vector<Blob<Dtype>*>* top) {
  const Dtype* bottom_data = bottom[0]->cpu_data();
  Dtype* top_data = (*top)[0]->mutable_cpu_data();
  int num = bottom[0]->num();
  int dim = bottom[0]->count() / bottom[0]->num();
  for (int i = 0; i < num; ++i) {
    std::vector<std::pair<int, Dtype> > bottom_data_vector;
    for (int j = 0; j < dim; ++j) {
      bottom_data_vector.push_back(std::make_pair(j, bottom_data[i * dim + j]));
    }
    std::partial_sort(
        bottom_data_vector.begin(), bottom_data_vector.begin() + top_k_,
        bottom_data_vector.end(), int_Dtype_pair_greater<Dtype>);
    if (out_max_val_) {
      for (int j = 0; j < top_k_; ++j) {
        top_data[(*top)[0]->offset(i, 0, j)] = bottom_data_vector[j].first;
        top_data[(*top)[0]->offset(i, 1, j)] = bottom_data_vector[j].second;
      }
    } else {
      for (int j = 0; j < top_k_; ++j) {
        top_data[(*top)[0]->offset(i, 0, j)] = bottom_data_vector[j].first;
      }
    }
  }
  return Dtype(0);
}

INSTANTIATE_CLASS(ArgMaxLayer);

}  // namespace caffe
