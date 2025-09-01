

#include <Unpacker.h>

#include <TAnalyzer.h>
#include <Histogramer.h>

#include <TCorrelator.h>

#include <TFile.h>
#include <TTree.h>


Unpacker* Unpacker::fUnpacker = 0;

Unpacker* Unpacker::Get() {
  if(!fUnpacker) 
    fUnpacker = new Unpacker();
  return fUnpacker;

}

Unpacker::Unpacker() : fIn(0), fOut(0) { } 

Unpacker::~Unpacker() { } 

std::mutex UnpackerMtx; 

void Unpacker::push(TFDSi &fdsi) {
  std::lock_guard<std::mutex> lock(UnpackerMtx);
  fQueue.push(fdsi); 
  fIn++;
  return;
}
    
TFDSi Unpacker::pop() {
  std::lock_guard<std::mutex> lock(UnpackerMtx);
  if(fQueue.size()!=0) {
    TFDSi fdsi = fQueue.front();
    fQueue.pop();
    fOut++;
    return fdsi;
  }
  TFDSi fdsi;
  return fdsi;
}



void Unpacker::Unpack() {

  TAnalyzer::Get(); 

  fLoopRunning = true;

  while(true) { 
    if(!TAnalyzer::Get()->LoopRunning() && TAnalyzer::Get()->qsize()==0)
      break;
    
    std::vector<ddasHit> hits = TAnalyzer::Get()->pop();

    TFDSi fdsi;
    fdsi.Reset();

    int counter[4] = {0}; 
    int pix=0;

    //loop one - find the dynodes.
    // -- a good time between anode and dynode is required in unpack anode.
    //double START=-1;
    
    int ndynode = 0;
    int gdynode = 0;
    int nanode  = 0;
    int ganode  = 0;

    for(ddasHit hit : hits) {
      Histogramer::fill("summary",4000,0,64000,hit.GetEnergy(),
                                  420,0,420,hit.GetId());
      fdsi.fClock.current = hit.GetTime();
      fdsi.fHitMap.push_back(hit.GetId());
      if(fdsi.fClock.initial<0) fdsi.fClock.initial = fdsi.fClock.current; 
      switch(hit.GetId()) {
        case 0: // PSPMT 1 high gain
          fdsi.fHighGain1.UnpackDynode(hit);
          ndynode++;
          break;
        case 1: // PSPMT 1 low gain
          fdsi.fLowGain1.UnpackDynode(hit);
          ndynode++;
          break;
        case 208: // PSPMT 2 high gain
          fdsi.fHighGain2.UnpackDynode(hit);
          gdynode++;
          break;
        case 209: // PSPMT 2 low gain
          fdsi.fLowGain2.UnpackDynode(hit);
          gdynode++;
          break;
        case 16: // anode high
        case 17:
        case 18:
        case 19:
        case 20: // anode low
        case 21:
        case 22:
        case 23:
          nanode++;
          break;

        case 212:  // anode
        case 213:
        case 214:
        case 215:
        case 216:  // anode 
        case 217:
        case 218:
        case 219:
          ganode++;
          break;
        default:
          break;
      };
    }
    Histogramer::fill("buildTime",10000,-5000,5000,fdsi.fClock.current - fdsi.fClock.initial);
    
    Histogramer::fill("nDynodeAnode",100,0,100,ndynode,100,0,100,nanode);
    Histogramer::fill("gDynodeAnode",100,0,100,gdynode,100,0,100,ganode);

    Histogramer::fill("DynodeDynode",100,0,100,ndynode,100,0,100,gdynode);
    Histogramer::fill("AnodeAnode",100,0,100,nanode,100,0,100,ganode);


    double previoustime = 0;

    for(ddasHit hit : hits) {
      if(previoustime==0) 
        previoustime = hit.GetTime();
      else 
        previoustime = fdsi.fClock.current;
      fdsi.fClock.current = hit.GetTime();

      switch(hit.GetId()) {
        case 0:  //dynode - handled above
        case 1:
          break;
        case 2:  //  Cross Scint B1
          if(!fdsi.nSIPMT.Hit())
            fdsi.gSIPM.Unpack(hit);
          break;
        case 4:  //  Cross Scint T1
          if(!fdsi.nSIPM.Hit())
            fdsi.gSIPM.Unpack(hit);
          break;
        case 6:  // MSX40
          fdsi.nPin1.Unpack(hit);
          //beta.pin01.Unpack(hit,beta.pin01_cal,0);
          break;
        case 7:  // vetoF
          fdsi.nVetoF.Unpack(hit);
          break;
        case 8:  // vetoB 
          fdsi.nVetoR.Unpack(hit);
          break;
        case 16: // anode high
        case 17:
        case 18:
        case 19:
          pix = hit.GetId()-16;
          fdsi.fHighGain1.UnpackAnode(hit,pix);
          break;
        case 20: // anode low
        case 21:
        case 22:
        case 23:
          pix = hit.GetId()-20;
          fdsi.fLowGain1.UnpackAnode(hit,pix);
          break;
        case 27:  //MSX100
          fdsi.nPin2.Unpack(hit);
          //beta.xpin2.Unpack(hit,beta.xpin2_cal,0);
          //beta.pin02.Unpack(hit,beta.pin02_cal,0);
          break;
        case 28:   /// DB4 PPAC Left
          break;
        case 29:   /// DB4 PPAC Tight
          break;
        case 30:   /// MSX40 Logic
          break;
        case 31:   /// DB3 SCINT LEFT
          break;
        case 32 ... 207: // vandle.
          fdsi.fVandle.Unpack(hit);
          break;
        case 208:  // DYNODE (handled above)
        case 209:  
          break;

        case 212:  // anode
        case 213:
        case 214:
        case 215:
          pix = hit.GetId()-212;
          fdsi.fHighGain2.UnpackAnode(hit,pix); // NO QDC!
          break;
        case 216:  // anode 
        case 217:
        case 218:
        case 219:
          pix = hit.GetId()-216;
          fdsi.fLowGain2.UnpackAnode(hit,pix); // NO QDC!
          break;
        case 229: //VETO FRONT   - switch to neutron crate
          fdsi.gVetoF.Unpack(hit);
          break;
        case 230: //VETO REAR    - switch to neutron crate
          fdsi.gVetoR.Unpack(hit);
          break;
        case 231: // Cross SCINT B2 Logic
          if(!fdsi.gSIPMT.Hit())
            fdsi.gSIPMT.Unpack(hit);
          break;
        case 232:  // Cross SCINT T@ 
          if(!fdsi.gSIPM.Hit())
            fdsi.gSIPM.Unpack(hit);
          break;
        case 233:  // MSX40 logic
          break;
        case 240: // pin01 logic?
          //fdsi.gPin1.Unpack(hit);
          break;
        case 241: // pin02 --- logic?
          //fdsi.gPin2.Unpack(hit);
          break;
        case 242: // DB3 PPAC Up Anode
          break;
        case 243: // DB3 PPAC UP Left
          break;

        case 244: // DB3 PPAC UP Right
          break;
        case 245: // DB3 PPAC Down Anode
          fdsi.gSCLT.Unpack(hit);
          break;
        case 246: // DB3 PPAC Down Left
          break;
        case 247: // DB3 PPAC Down Right
          break;
        case 248: //sipmt  ---  DB4 PPAC Up 
                  //beta.SIPMT.Unpack(hit,beta.SIPMT_cal,0);
          break;
          //case 234: //sclt
        case 249:  // DB4 PPAC Down
          break;
        case 250:  // DB4 PPAC Left
          break;
        case 251:  // DB4 PPAC Right
          break;
        case 252:  // DB5 PPAC Right
          break;
        case 253:  // DB5 PPAC Left
          break;
        case 254:  // DB5 PPAC Down Left
          break;
        case 255:  // DB$ PPAC Down Right
          break;
        case 256: // Clover 1 A  //start of slot 3
        case 257: // Clover 1 B
        case 258: // Clover 1 C
        case 259: // Clover 1 D
        case 260: // Clover 2 A
        case 261: // Clover 2 B
        case 262: // Clover 2 C
        case 263: // Clover 2 D
        case 264: // Clover 3 A
        case 265: // empty 
        case 266: // empty
        case 267: // Clover 3 D
        case 268: // empty

        case 272: // Clover 5 A  //start of slot 4 
        case 273: // Clover 5 B
        case 274: // Clover 5 C
        case 275: // CLover 5 D
        case 276: // Clover 6 A
        case 277: // Clover 6 B
        case 278: // Clover 6 C
        case 279: // Clover 6 D
        case 280: // empty
        case 281: // empty
        case 282: // empty 
        case 283: // Clover 7 D
        case 284: // empty

        case 288: // Clover 9 A  //start of slot 5
        case 289: // Clover 9 B
        case 290: // Clover 9 C
        case 291: // Clover 9 D
        case 292: // Clover 10 A
        case 293: // Clover 10 B
        case 294: // Clover 10 C
        case 295: // Clover 10 D
        case 296: // Clover 11 A
        case 297: // Clover 11 B
        case 298: // Clover 11 C
        case 299: // Clover 11 D
        case 300: // empty
        case 301: // empty 
        case 302: // Clover 12 C
        case 303: // Clover 12 D

        case 304: // Clover 13 A //start of slot 6
        case 305: // Clover 13 B
        case 306: // Clover 13 C
        case 307: // Clover 13 D
          fdsi.fClover.Unpack(hit,hit.GetId()-256);
          break;
        case 308: // Clover 3 B - 265
        case 309: // Clover 3 C - 266
          fdsi.fClover.Unpack(hit,hit.GetId()-256-43);
          break;
        case 310: // Clover 7 A - 280
        case 311: // Clover 7 B - 281
        case 312: // Clover 7 C - 282
          fdsi.fClover.Unpack(hit,hit.GetId()-256-30);
          break;
        case 313:
        case 314:
        case 315:
          break;

        case 330: // ???????
          break;

        case 336: //             //start of slot 8
          break;
        case 340: // Clover 12 A - 300
        case 341: // Clover 12 B - 301
          fdsi.fClover.Unpack(hit,hit.GetId()-256-40);
          break;

        case 352: // pin01 logic?
          fdsi.gPin1.Unpack(hit);
          break;
        case 353: // pin02 --- logic?
          fdsi.gPin2.Unpack(hit);

        default:
          break;
      };
    }
 
    // check fdsi, add to queue....
    if(fdsi.fHighGain1.dyhit==1 || fdsi.fHighGain2.dyhit==1) { 
       fdsi.fHighGain1.hit = 1;
       fdsi.fHighGain1.FitAnodePosition();
       fdsi.fHighGain2.hit = 1;
       fdsi.fHighGain2.FitAnodePosition();
    }
    if(fdsi.fLowGain1.dyhit==1 || fdsi.fLowGain2.dyhit==1) { 
       fdsi.fLowGain1.hit = 1;
       fdsi.fLowGain1.FitAnodePosition();
       fdsi.fLowGain2.hit = 1;
       fdsi.fLowGain2.FitAnodePosition();
    }         
    fdsi.fEventType = fdsi.EventType();
    Histogramer::fill("EventType",100,0,100,fdsi.EventType());

    //set the PID...
    fdsi.fPID.de1 =  fdsi.nPin1.Ecal();
    fdsi.fPID.de2 =  fdsi.nPin2.Ecal();
    fdsi.fPID.tof =  fdsi.gSIPMT.Time() - fdsi.gSCLT.Time();
    fdsi.fPID.xpos = fdsi.fLowGain1.xpos;
    fdsi.fPID.ypos = fdsi.fLowGain1.ypos;
    fdsi.fPID.time = fdsi.nPin1.Time();
    fdsi.fPID.hasGoodPosition = fdsi.fPID.goodPosition();

    push(fdsi);
  }


  fLoopRunning = false;
}


std::string Unpacker::Status() {
  std::lock_guard<std::mutex> lock(UnpackerMtx);
  std::string s = Form("Unpacker:  in[%lu]  out[%lu]  q[%lu]",fIn, fOut, qsize());
  fIn = 0;
  fOut =0;
  return s;
}



