
#include <TOFCorrector.h>

#include <TCanvas.h>
#include <TVirtualPad.h>

#include <TFile.h>
#include <TSpectrum.h>
#include <TSpline.h>
#include <TProfile.h>
#include <TH2.h>
#include <TRandom.h>

#include <utils.h>

TOFCorrector::TOFCorrector() : fTofTime(0) { }

TOFCorrector::TOFCorrector(std::string fname) : fTofTime(0) { 
  TDirectory *current =gDirectory;
  TFile *temp = TFile::Open(fname.c_str());
  TOFCorrector *tc = (TOFCorrector*)temp->Get("TOFCorrector");
  tc->Copy(*this);
  temp->Close();
  current->cd();
}

TOFCorrector::~TOFCorrector() { }


void TOFCorrector::Copy(TOFCorrector &other) const { 
  TObject::Copy(other);

  other.fNPeaks        = fNPeaks;
  other.fExpectedBin   = fExpectedBin;
  other.fExpectedValue = fExpectedValue;
  other.fSplines        = fSplines;
  //if(fTofTime)
  other.fTofTime = fTofTime;

}



void TOFCorrector::FitTOF(TH2 *tof_time) {
  if(!tof_time)
    tof_time =fTofTime;    

  TH1D *py      = fTofTime->ProjectionY();

  TSpectrum *s = new TSpectrum;
  double threshold = 0.0005;
  int iterations =0;
  s->Search(py,1,"goff",threshold);

  while(s->GetNPeaks()!=EXPECTED_PEAKS && iterations<1000) {
    if(s->GetNPeaks()>EXPECTED_PEAKS)
      threshold+=threshold*.005;
    else  
      threshold-=threshold*.005;
    s->Search(py,1,"goff",threshold);
  }

  if(s->GetNPeaks()!=EXPECTED_PEAKS)
    return;


  for(int i=0;i<s->GetNPeaks();i++) {  
    //peaks.push_back(std::make_pair(s->GetX()[i],py->GetXaxis()->FindBin(s->GetX()[i])));

    double ntof = s->GetPositionX()[i];
    int   btof = py->GetXaxis()->FindBin(ntof);
    int   ltof = btof-TOF_WIDTH/2;
    int   htof = btof+TOF_WIDTH/2;
    //TH1D *p = tof->ProjectionX(Form("p%i",i+1),ltof,htof);
    TProfile *p = fTofTime->ProfileX(Form("p%i",i+1),ltof,htof);

    TGraph gr;
    for(int i=1;i<=p->GetNbinsX();i++) { 
      if(int(p->GetBinContent(i))!=0) {
        gr.AddPoint(p->GetBinCenter(i),p->GetBinContent(i));
      }
    }
    TSpline3 *spline = new TSpline3(Form("sp%i",i+1),gr.GetX(),gr.GetY(),gr.GetN());

    spline->SetLineWidth(2);
    spline->SetLineColor(2);
    spline->SetNpx(100000);
    //splines3->Draw("same");
    
    //new TCanvas;
    //gr.DrawClone("A*");
    //p->DrawClone("same");
    //spline->Draw("same");

    fExpectedBin.push_back(btof);
    fExpectedValue.push_back(ntof);
    fSplines.push_back(spline);

    printf("found peak %.1f:\t with limits %i to %i\n",ntof,ltof,htof);
  }
  fNPeaks =s->GetNPeaks();

}



void TOFCorrector::MakeCorrectionFile(std::string fname, TH2  *tof_time) { 

  TFile *f =0;
  //if(fname.length()==0)
  //  f =_file0;
  //else
  f = TFile::Open(fname.c_str());

  int run,subrun;
  getRunNumber(fname,run,subrun);
  printf("fname: %s\n",fname.c_str());
  printf("run: %i\n",run);
  printf("subrun: %i\n",subrun);

  if(!f)
    return;

  if(!tof_time)
    tof_time = (TH2*)f->Get("tof_time");  //this is a 2d: xaxis runtime, yaxis TOF.

  fTofTime = (TH2*)tof_time->Clone(Form("%s_clone",tof_time->GetName()));

  FitTOF(fTofTime);

  TDirectory *current =gDirectory;
  TFile *ofile = TFile::Open(Form("tof%04i-%02i.tof",run,subrun),"recreate");
  this->Write();
  ofile->Close();
  current->cd();
} 

double TOFCorrector::Correct(double tof,double time,int peak) const {
  // time should be in seconds since run start....
  TSpline3 *sp = fSplines.at(peak);
  double offset = sp->Eval(time);
  double correction = PEAK1-offset;
  return tof+correction;
}

TH2* TOFCorrector::CorrectedTOF() const {
  if(!fTofTime)
    return 0;
  TH2* tofCorrected = (TH2*)fTofTime->Clone("tofCorrected_uncorrected");
  tofCorrected->Reset();
  for(int x=1;x<=fTofTime->GetNbinsX();x++) {
    float ts = fTofTime->GetXaxis()->GetBinLowEdge(x); 
    for(int y=1;y<=fTofTime->GetNbinsY();y++) {
      float binContent = fTofTime->GetBinContent(x,y);
      float tof = fTofTime->GetYaxis()->GetBinLowEdge(y);
      int z=0;
      while(z<binContent) {
        float tx = ts  + gRandom->Uniform(0,fTofTime->GetXaxis()->GetBinWidth(1));
        float ty = tof + gRandom->Uniform(0,fTofTime->GetYaxis()->GetBinWidth(1));
        tofCorrected->Fill(tx,Correct(ty,tx));
        z++;
      }
    }  
  }
  return tofCorrected;
}


void TOFCorrector::Draw() const {
  if(!fTofTime) return;

  TCanvas *g = new TCanvas();
  g->Divide(2,1);

  
  TVirtualPad *p1 = g->cd(1);
  p1->Divide(1,2);
  p1->cd(1);
  fTofTime->Draw("colz2"); 
  p1->cd(2);
  CorrectedTOF()->Draw("colz2");

  TVirtualPad *p2 = g->cd(2);
  p2->Divide(1,fNPeaks);
  for(int i=0;i<fNPeaks;i++) {
    p2->cd(i+1);
    int b = fExpectedBin.at(i);
    TProfile *prof = fTofTime->ProfileX(Form("fTofTime_%i",i+1),b-TOF_WIDTH/2,b+TOF_WIDTH/2);
    prof->Draw();
    double tof = fExpectedValue.at(i);
    prof->GetYaxis()->SetRangeUser(tof-5,tof+5);
    fSplines.at(i)->Draw("same");
  }
};

