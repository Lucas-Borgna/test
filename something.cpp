#include <iostream>
#include <fstream>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include "Event.h"
#include <TMVA/Reader.h>
#include <sstream>
#include <string>
#include <cstdlib>

//Need to convert int 2 sting (normal C++11 function doesnt work)
template <typename T>
    std::string NumberToString(T Number){
        std::ostringstream ss;
        ss << Number;
        return ss.str();
    }


int main() {

// Set up an output file and book some histograms

  TFile* histFile = new TFile("analysis.root", "RECREATE");
  //TH1D* hFishSig = new TH1D("hFishSig", "MLP", 100, -2., 2.);
  //TH1D* hFishBkg = new TH1D("hFishBkg", "MLP", 100, -2., 2.);
  TH1D* hBDT200Sig = new TH1D("hBDT200Sig", "BDT200", 100, -2.0, 2.0);
  TH1D* hBDT200Bkg = new TH1D("hBDT200Bkg", "BDT200", 100, -2.0, 2.0);

  //Q2b - Write output to text file for later analysis.
  ofstream output_file;
  output_file.open("training_error_rates.txt");
  if (output_file.fail()){
    std::cout << "unable to open file error_rates.txt" << std::endl;
    exit(1);
   }

  //Array of BDT iterations
  int numBDT[12] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 1000, 50000};

  for (int j = 0; j < 1; j++){

// Set up the TMVA Reader object.
// The names in AddVariable must be same as in the input (weight) file.

  TMVA::Reader* reader = new TMVA::Reader();
  float x, y, z;                       // TMVA needs float, not double
  reader->AddVariable("x", &x);  
  reader->AddVariable("y", &y);
  reader->AddVariable("z", &z);
  std::string dir    = "../train/weights/";
  std::string prefix = "tmvaTest";

  std::string BDT_string = NumberToString(numBDT[j]);

  //reader->BookMVA("BDT" + BDT_string, dir + prefix + "_BDT" + BDT_string + ".weights.xml");
    reader -> BookMVA("BDT200", dir + prefix + "_BDT200.weights.xml");

// Open input file, get the TTrees, put into a vector

  TFile* inputFile = new TFile("../generate/testData.root");
  //TFile* inputFile = new TFile("../generate/trainingData.root");
  inputFile->ls();
  TTree* sig = dynamic_cast<TTree*>(inputFile->Get("sig"));
  TTree* bkg = dynamic_cast<TTree*>(inputFile->Get("bkg"));
  std::vector<TTree*> treeVec;
  treeVec.push_back(sig);
  treeVec.push_back(bkg);

// Loop over TTrees

  int nSigAccFish = 0;
  int nBkgAccFish = 0;
  int nSig, nBkg;
  const double tCutFisher = 0.0;
  const double tCutMLP = 0.5;
  double sig_count = 0;
  double bkg_count = 0;
  for (int i=0; i<treeVec.size(); i++){

    treeVec[i]->Print();
    Event evt;
    treeVec[i]->SetBranchAddress("evt", &evt);
    int numEntries = treeVec[i]->GetEntries();
    if ( i == 0 ) { nSig = numEntries; }
    if ( i == 1 ) { nBkg = numEntries; }
    std::cout << "number of entries = " << numEntries << std::endl;

// Loop over events.  The test statistic is identified by the first 
// argument used above in BookMVA (below, e.g., "Fisher").

    for (int j=0; j<numEntries; j++){
      treeVec[i]->GetEntry(j);                // sets evt
      x = evt.x;                              // set variables of reader
      y = evt.y;
      z = evt.z;

      double tBDT = reader -> EvaluateMVA("BDT200");
      //double tBDT = reader->EvaluateMVA("BDT" + BDT_string);
      if ( i == 0 ){                       // signal
        //hFishSig->Fill(tFisher);
        //hFishSig -> Fill(tBDT);
        hBDT200Sig -> Fill(tBDT);
        if (tBDT < 0.0 ){
            sig_count++;
        }

      }
      else if ( i == 1 ){                  // background
        //hFishBkg->Fill(tFisher);
        hBDT200Bkg -> Fill(tBDT);
        if (tBDT > 0.0){
            bkg_count++;
        }

      }

      // ADD YOUR CODE HERE TO INCLUDE OTHER CLASSIFIERS SUCH AS MLP
      // AND COUNT THE NUMBERS OF SELECTED EVENTS (COMPARE CLASSIFIER
      // VALUE TFISHER, ETC., TO TCUT).


    }

    std::cout << "Signal Count = " << sig_count << std::endl;
    std::cout << "Backgrounds = " << bkg_count << std::endl;

    //hFishSig -> Draw();
    //hFishBkg -> Draw("Same");
    hBDT200Sig -> Draw();
    hBDT200Bkg -> Draw("same");

  }
  double total_events = 10000.0;
  double error_rate_total = (sig_count/total_events) + (bkg_count/total_events);

  std::cout << "total error rate = " <<  error_rate_total << std::endl;
  output_file << error_rate_total << std::endl;
 }

  histFile->Write();
  histFile->Close();

  //std::cout << "sig_count = " << sig_count << std::endl;
  //std::cout <<"bkg_count  = " << bkg_count << std::endl;

  return 0;


}