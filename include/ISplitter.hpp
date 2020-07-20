#ifndef ISPLITTER_H_
#define ISPLITTER_H_

#include <memory>
#include <vector>
#include <deque>
#include <random>
#include <map>
#include <random>
#include <mutex>
#include <iterator>
#include <queue>
#include <atomic>  


class ISplitter
{
  public:
    
    ISplitter(/*[in]*/ int _nMaxBuffers, /*[in]*/ int _nMaxClients);    
    ISplitter() {};    
    
    virtual ~ISplitter();

    bool SplitterInfoGet(/*[out]*/ int* _pnMaxBuffers, 
                         /*[out]*/ int* _pnMaxClients);
    
    int  SplitterPut( /*[in]*/ const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, 
                       /*[in]*/ int _nTimeOutMsec);
    
    int  SplitterFlush();
    bool SplitterClientAdd(/*[out]*/ int* _pnClientID);
    bool SplitterClientRemove(/*[in]*/ int _nClientID);

    bool SplitterClientGetCount(/*[out]*/ int* _pnCount);
    bool SplitterClientGetByIndex(/*[in]*/ int _nIndex, /*[out]*/ int* _pnClientID, /*[out]*/ int* _pnLatency);
    
    int   SplitterGet(/*[in]*/ int _nClientID, 
                     /*[out]*/ std::shared_ptr<std::vector<uint8_t>>& _pVecGet, 
                     /*[in]*/ int _nTimeOutMsec);
    
    void SplitterClose();
    
    // tmp  
    std::deque<std::shared_ptr<std::vector<uint8_t>>> getBuffer() {return __buffer;};
  private:

    std::deque<std::shared_ptr<std::vector<uint8_t>>> __buffer;
    std::map<int, int> __clients; // map<ID, count>
    
    std::deque<int> __qClients; // <ID, time>
    std::atomic<int> __client;
    
    std::queue<long> __qSupervisor; // <ID>
    std::atomic<long> __supervisor;
    std::atomic<bool> __isSupervisor;
    
    int __nMaxBuffers;
    int  __nMaxClients;
    
    std::mutex __mtx;  
    std::mutex __mtxQC;  
    std::mutex __mtxQS;  

};
#endif
