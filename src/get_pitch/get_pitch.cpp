/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -h, --help  Show this screen
    --version   Show the version of the project
    --window <type>  Window type [default: RECT]
    --rmax FLOAT   Rmax (r[max]/r[0]) threshold for voiced/unvoiced sound [default: 0.7]
    --r1 FLOAT     R1 (r[1]/r[0]) threshold for voiced/unvoiced sound [default: 0.7]
    --pow FLOAT    Power threshold for voiced/unvoiced [default: -40]
    --center FLOAT Center clip threshold [default: 0.01]

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();
  std::string window = args["--window"].asString();

  float RMAX_THRSHLD = stof(args["--rmax"].asString());
  float R1_THRSHLD = stof(args["--r1"].asString());
  float POW_THRSHLD = stof(args["--pow"].asString());
  float CENTER_CLIP_THRESHOLD = stof(args["--center"].asString());

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer::Window window_type;
  if (window == "RECT") {
    window_type = PitchAnalyzer::RECT;
  } else if (window == "HAMMING") {
    window_type = PitchAnalyzer::HAMMING;
  } else {
    cerr << "Invalid window type\n";
    return -1;
  }
  PitchAnalyzer analyzer(n_len, rate, window_type, 50, 500, RMAX_THRSHLD, R1_THRSHLD, POW_THRSHLD);

  // Preprocess the input signal
  for(unsigned int i=0; i<x.size(); i++){
    if(abs(x[i])<CENTER_CLIP_THRESHOLD){
      x[i]=0;
    }
  }
  
  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  // Median Filter
  vector<float> vect;
  vector<float> filtered_f0(f0.size());
  filtered_f0[0] = f0[0];
  for (unsigned int i = 1; i < f0.size() - 1; i++) {
    vect = {f0[i - 1], f0[i], f0[i + 1]};
    sort(vect.begin(), vect.end());
    filtered_f0[i] = vect[1];
  }
  filtered_f0[f0.size() - 1] = f0[f0.size() - 1];
  f0 = filtered_f0;

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
