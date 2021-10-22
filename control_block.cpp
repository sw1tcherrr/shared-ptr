#include "control_block.h"
using namespace shared_ptr_detail;

void control_block::dec_strong() {
  if (--strong_cnt == 0) {
    delete_object();
  }
  dec_weak();
}

void control_block::inc_strong() {
  ++strong_cnt;
  ++weak_cnt;
}

void control_block::dec_weak() {
  if (--weak_cnt == 0) {
    delete this;
  }
}

void control_block::inc_weak() {
  ++weak_cnt;
}

size_t control_block::use_count() const {
  return strong_cnt;
}
