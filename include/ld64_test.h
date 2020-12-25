
#ifndef __LD64_TEST_H__
#define __LD64_TEST_H__

class Timer {
 public:  
  Timer(): started_(false), paused_(false) {}

  void Start() {
    started_ = true;
    paused_ = false;
    start_time_ = std::chrono::steady_clock::now();
  }
  
  void Restart() {
    started_ = false;
    Start();
  }
  
  void Pause() {
    paused_ = true;
    pause_time_ = std::chrono::steady_clock::now();
  }
  
  void Resume() {
    paused_ = false;
    start_time_ += std::chrono::steady_clock::now() - pause_time_;
  }
  
  void Reset() {
    started_ = false;
    paused_ = false;
  }

  double ElapsedMicroSeconds() const {
    using namespace std::chrono;
    
    if (!started_) return 0.0;
    if (paused_)
      return duration_cast<microseconds>(pause_time_ - start_time_).count();
    else
      return duration_cast<microseconds>(steady_clock::now() - start_time_).count();
  }

 private:
  bool started_;
  bool paused_;
  std::chrono::steady_clock::time_point start_time_;
  std::chrono::steady_clock::time_point pause_time_;
};

std::string opcodes(const uint8_t* ptr, const size_t length) {
  std::stringstream stream;  
  
  for (size_t i = 0; i < length; ++i) {
    stream << std::setfill ('0') << std::setw(2)
           << std::hex << (static_cast<int>(ptr[i]) & 0xff);
           
    if (i < length-1) stream << " ";
  }
  
  return stream.str();
}

#endif // __LD64_TEST_H__
