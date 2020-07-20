#include <ISplitter.hpp>
#include <iostream> // tmp

ISplitter::ISplitter(int _nMaxBuffers, int _nMaxClients)
{
    __nMaxBuffers = _nMaxBuffers;
    __nMaxClients =  _nMaxClients;
}    
ISplitter::~ISplitter() 
{
}
bool ISplitter::SplitterInfoGet(int* _pnMaxBuffers, int* _pnMaxClients) 
{ 
  *_pnMaxBuffers = __nMaxBuffers;
  *_pnMaxClients = __nMaxClients;
  return true;
}

int ISplitter::SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int _nTimeOutMsec)
{
  auto t1 = std::chrono::high_resolution_clock::now();
  int result = 0;
  long ID = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  __mtxQS.lock(); 
  __qSupervisor.push(ID);  
  __supervisor  =  __qSupervisor.front(); 
  __isSupervisor  =  true; 
  __mtxQS.unlock();
  
  bool isWrite = false;
  std::vector<int> outsiders;
  int maxCount = 0; 
  __mtx.lock();
  for(auto& it: __clients) {
    if(maxCount < it.second) maxCount =  it.second;
    if(__nMaxBuffers == it.second)
      outsiders.push_back(it.first);
  }
  if(maxCount < __nMaxBuffers && __supervisor == ID) { 
    if((int)__buffer.size() == __nMaxBuffers && __buffer.size() > 0)
      __buffer.pop_front();
    __buffer.push_back(_pVecPut);
    for(auto& it: __clients)
      it.second = it.second + 1;
    isWrite = true;
  } else isWrite = false;
  __mtx.unlock();
  
  if(isWrite) {
    __mtxQS.lock(); 
    __qSupervisor.pop();  
    if( __qSupervisor.size() > 0) {
      __supervisor  =  __qSupervisor.front(); 
      __isSupervisor  =  true;
    }
    else __isSupervisor  =  false; 
    __mtxQS.unlock();
    result = 0;
  }
  
  __isSupervisor  =  false; 
  auto t2 = std::chrono::high_resolution_clock::now();
  while(!isWrite && std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < _nTimeOutMsec) {  
    for(unsigned int i = 0;  i < outsiders.size(); i++) {
      if(__client == outsiders.at(i))
          outsiders.erase(outsiders.begin()+i);
    }
    if(outsiders.size() == 0 && __supervisor == ID) {
      __isSupervisor  =  true; 
      __mtx.lock();
      __buffer.pop_front();
      __buffer.push_back(_pVecPut);
      for(auto& it: __clients)
        it.second = it.second + 1;
      isWrite = true;
      __mtx.unlock();
    }
    if(isWrite) {
      __mtxQS.lock(); 
      __qSupervisor.pop();  
      if( __qSupervisor.size() > 0) {
        __supervisor  =  __qSupervisor.front(); 
        __isSupervisor  =  true;
      }
      else __isSupervisor  =  false; 
      __mtxQS.unlock();
    result = 0;
    }  
  t2 = std::chrono::high_resolution_clock::now();
  }
 
  if(!isWrite) {
    __mtx.lock();
    __buffer.pop_front();
    __buffer.push_back(_pVecPut);
    for(auto& it: __clients) {
      if(it.second < __nMaxBuffers)
        it.second = it.second + 1;
    }
    __mtx.unlock();
    
    __mtxQS.lock(); 
    __qSupervisor.pop();  
    if( __qSupervisor.size() > 0) {
      __supervisor  =  __qSupervisor.front(); 
      __isSupervisor  =  true;
    }
    else __isSupervisor  =  false; 
    __mtxQS.unlock();
    result = 1;
  }
  //return result; // tmp
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

int ISplitter::SplitterFlush() 
{
  bool isRun = true;
  long ID = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  __mtxQS.lock(); 
  __qSupervisor.push(ID);  
  __supervisor  =  __qSupervisor.front(); 
  __isSupervisor  =  true; 
  __mtxQS.unlock();
  while(isRun) {
      if( __supervisor == ID) {
      __mtx.lock();
      __buffer.clear();
      for(auto& it: __clients)
        it.second = -1;
      __mtx.unlock();
      isRun = false;
      }
  }
    __mtxQS.lock(); 
    __qSupervisor.pop();  
    if( __qSupervisor.size() > 0) {
      __supervisor  =  __qSupervisor.front(); 
      __isSupervisor  =  true;
    }
    else __isSupervisor  =  false; 
    __mtxQS.unlock();

  return 0;
}

bool ISplitter::SplitterClientAdd(int* _pnClientID) 
{
  int ID = static_cast<int>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  *_pnClientID = ID;
  bool result = false;
  __mtx.lock();
  if(__clients.find(ID) != __clients.end()) result = false;
  else {
    if((int)__clients.size() < __nMaxClients) {
      __clients.insert(std::pair<int, int>(*_pnClientID, 0));
      result = true;
    }
    else result = false;
  }  
  __mtx.unlock();
  return  result;
  
} 

bool ISplitter::SplitterClientRemove(int _nClientID)
{
  bool isDeleted = false;
  __mtx.lock();
  auto it = __clients.find(_nClientID);
  if(it != __clients.end()) {
    __clients.erase(it);
    isDeleted = true;
  }
  __mtx.unlock();

  return isDeleted;
}

bool ISplitter::SplitterClientGetCount(int* _pnCount) // ?
{
  return false;
}
bool ISplitter::SplitterClientGetByIndex(int _nIndex, int* _pnClientID, int* _pnLatency) // !
{
  // Need to fix!
  bool isFound = false;
  int size = 0; 
__mtx.lock();
  size = __clients.size();
  if(_nIndex >= __nMaxClients || _nIndex >= size) isFound = false;
  else {
    int n = 0;
    for(auto& it: __clients) {
      if(n ==  _nIndex) {
        *_pnClientID = it.first;
        *_pnLatency = it.second;
        isFound = true;
        break;
      }
      n++;
    }
  }
  __mtx.unlock();
  return isFound;
}

int ISplitter::SplitterGet(int _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int _nTimeOutMsec)
{
  auto t1 = std::chrono::high_resolution_clock::now();
  
  int err = 0;
  int count = 0;
  std::map<int,int>::iterator it;
  bool isRun = true;
 
  __mtxQC.lock(); 
  __qClients.push_back(_nClientID);  
  __client  =  __qClients.front(); 
  __mtxQC.unlock();
  
  auto t2 = std::chrono::high_resolution_clock::now();
  while(isRun && 
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < _nTimeOutMsec) {
    if(__client == _nClientID && !__isSupervisor) {
      
      __mtx.lock();
      it = __clients.find(_nClientID);
      if(it == __clients.end())  {
        err = 1;
        isRun = false;
      }
      else {
        count = it->second;
        if(count <= 0) t2 = std::chrono::high_resolution_clock::now();
        else {
          _pVecGet = __buffer.at(__buffer.size() - count);
          it->second =  it->second - 1;
          isRun = false;      
        }
      } 
      __mtx.unlock();
      
      __mtxQC.lock(); 
      __qClients.pop_front();
      __client  =  __qClients.front(); 
      if(isRun) __qClients.push_back(_nClientID);  
      __mtxQC.unlock();
    }
    else t2 = std::chrono::high_resolution_clock::now();
  }
  if(isRun) {
    err = 3;
    __mtxQC.lock(); 
      for (auto i = __qClients.begin(); i != __qClients.end();) {
        if(*i == _nClientID) {
          i = __qClients.erase(i); 
          break;
        }
        else ++i;
      }
    __mtxQC.unlock();
  }
  return err;
}

void ISplitter::SplitterClose()
{
  std::cout << "Hello";
  bool isRun = true;
  long ID = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  __mtxQS.lock(); 
  __qSupervisor.push(ID);  
  __supervisor  =  __qSupervisor.front(); 
  __isSupervisor  =  true; 
  __mtxQS.unlock();
  while(isRun) {
      if( __supervisor == ID) {
      __mtx.lock();
      __buffer.clear();
      __clients.clear();
      __mtx.unlock();
      isRun = false;
      }
  }
    __mtxQS.lock(); 
    __qSupervisor.pop();  
    if( __qSupervisor.size() > 0) {
      __supervisor  =  __qSupervisor.front(); 
      __isSupervisor  =  true;
    }
    else __isSupervisor  =  false; 
    __mtxQS.unlock();
}
