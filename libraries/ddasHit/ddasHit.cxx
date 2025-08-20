#include<ddasHit.h>

#include<cmath>

ddasHit::ddasHit() { Clear(); }

ddasHit::~ddasHit() { } 


void ddasHit::Clear() {
  evId   = ULLONG_MAX;
  id     = INT_MAX;
  energy = sqrt(-1);
  time   = ULLONG_MAX;
  cfd    = INT_MAX;
  //for(int i=0;i<8;i++) qdc[i] = INT_MAX;
  qdc.clear();
  traceLength=0;
  trace.clear();

}

void ddasHit::print() const {
  printf("ddas hit @ %f\n", GetTime()+GetCFDTime()/16384.);
  printf("\tevId:    %llu\n",evId);
  printf("\tid:      %i\n",id);
  printf("\tenergy:  %.1f\n",energy);
  printf("\tqdcSize: %lu\n",qdc.size());
  if(qdc.size()>0) {
    printf("\t\t");
    for(size_t i=0;i<qdc.size();i++) 
      printf("[%lu]:%i ",i,qdc.at(i));
    printf("\n");
  }
  printf("\ttraceLength: %i\n",traceLength);
  if(traceLength) {
    printf("\t\t");
    for(int i=0;i<traceLength;i++) {
      if(i!=0 && (i%16==0))
        printf("\n\t\t");
      printf("%i  ",trace.at(i));
    }
    printf("\n");
  }
  printf("---------------------------------\n");
  fflush(stdout);
}

void ddasHit::Copy(ddasHit& lhs) const {

  lhs.evId = evId;
  lhs.id   = id;

  lhs.setEvId(evId);
  lhs.setId(id);
  lhs.setEnergy(energy);
  lhs.setTime(time);
  lhs.setCFD(cfd);
  lhs.setQDC(qdc);
  lhs.setTraceLength(traceLength);
  lhs.setTrace(trace);



}

ddasHit ddasHit::operator=(ddasHit const& rhs) {
  ddasHit hit;
  rhs.Copy(hit);
  return hit;
}

bool ddasHit::operator==(ddasHit const& rhs) {
  if( evId==rhs.evId && id==rhs.id) 
    return true;
  return false;
}

bool ddasHit::operator<(ddasHit const & rhs) const {
  //double right = rhs.time + rhs.cfd/16384.;
  //double left  = time + cfd/16384.;
  //return left<right;
  if(GetId() == rhs.GetId()) {
    double right = rhs.time + rhs.cfd/16384.;
    double left  = time + cfd/16384.;
    return left<right;
  }
  return GetId() <= rhs.GetId();
}

