#include <iostream>
#include <ISplitter.hpp>
#include <thread> 
#include <ctime> 

std::shared_ptr<ISplitter> SplitterCreate(/*[in]*/ int _nMaxBuffers, /*[in]*/ int _nMaxClients) {
  std::shared_ptr<ISplitter> splitter(new ISplitter(_nMaxBuffers,  _nMaxClients));
  return splitter;
}


int create_test(const std::shared_ptr<ISplitter>& splitter, int _nMaxBuffers,  int _nMaxClients, std::vector<int>* ids) 
{
  // ------
  int overClients = 4;
  int id = 0;
  std::cout << "Trying to add Clients N=" << _nMaxClients +  overClients << "!" << std::endl;  
  for(int i = 0; i < _nMaxClients + overClients; i++) {
    bool isID = splitter->SplitterClientAdd(&id);
    std::cout << "id:" << id << "|" << (int)isID << " ";
    ids->push_back(id);
  }
  std::cout << std::endl;
  
  // ------
  srand (time (NULL)); // + include ctime
  int deleteID =  rand () % (ids->size() - 1) + 1; 
  bool isDeleteID = splitter->SplitterClientRemove(ids->at(deleteID));
  std::cout << "What to delete ID: " << ids->at(deleteID)  << " -> " << "Result: " << isDeleteID << "!" <<  std::endl;  
  isDeleteID = splitter->SplitterClientRemove(deleteID);
  std::cout << "What to delete ID: " << deleteID  << " -> " << "Result: " << isDeleteID << "!" << std::endl;  
  // ------
  
  // ------
  std::cout << "Trying to get id all clients" << std::endl;  
  int client = -1;
  int latency = -1;
  for(int i = 0; i < _nMaxClients; i++) {
    bool is = splitter->SplitterClientGetByIndex(i, &client, &latency);
    if(is)
      std::cout << "id:" << client << "|" << "c:" << latency << " ";
  }
  std::cout << std::endl;

return 1;
}

int fill(const std::shared_ptr<ISplitter>& splitter, int nTimeOutMsec, int times) 
{
  uint8_t n = 1; int N = times;
  while(n < N) {
  std::shared_ptr<std::vector<uint8_t>> data(new std::vector<uint8_t>());
    data->push_back(n);
     int delay = splitter->SplitterPut(data, nTimeOutMsec); // tmp 
    std::this_thread::sleep_for(std::chrono::milliseconds(nTimeOutMsec*2 - delay)); n = n + 1;
    
    //for(unsigned int i = 0; i < data->size(); i++)
    //  std::cout << (int)data->at(i) << " ";
    //std::cout << std::endl;
  }

  return 1;  
} 

int get(const std::shared_ptr<ISplitter>& splitter, int nTimeOutMsec, int times, int ID, std::deque<std::shared_ptr<std::vector<uint8_t>>>* buf, int extra) 
{
  uint8_t n = 1; int N = times;
  while(n < N) {
    std::shared_ptr<std::vector<uint8_t>> data(new std::vector<uint8_t>());
     int isGet = splitter->SplitterGet(ID, data, nTimeOutMsec);
    if(!isGet) buf->push_back(data);
    std::this_thread::sleep_for(std::chrono::milliseconds(nTimeOutMsec)); n = n + 1;
    if(n == 5) std::this_thread::sleep_for(std::chrono::milliseconds(extra));
    //if(n == 7) splitter->SplitterFlush();
    //if(n == 7) splitter->SplitterClose();
    
    //for(unsigned int i = 0; i < data->size(); i++)
    //  std::cout << (int)data->at(i) << " ";
    //std::cout << std::endl;
  }

  return 1;  
} 
int main(void)
{
  int _nMaxBuffers = 2; int _nMaxClients = 10;
  std::shared_ptr<ISplitter> splitter =  SplitterCreate(_nMaxBuffers, _nMaxClients); 
  std::cout << "Splitter B=" << _nMaxBuffers << " C=" << _nMaxClients << std::endl;  
  
  std::vector<int> ids;
  int result = create_test(splitter,  _nMaxBuffers,  _nMaxClients, &ids);
  std::cout << "create_test: " << result << std::endl;
  

  int pulses_read = 30;
  int pulses_fill = 10;
  std::deque<std::shared_ptr<std::vector<uint8_t>>> cbuf;
  std::deque<std::shared_ptr<std::vector<uint8_t>>> cbuf2;
  std::deque<std::shared_ptr<std::vector<uint8_t>>> cbuf3;
   std::thread in_1 (fill, splitter, 50, pulses_fill +1); 
   //std::thread in_2 (fill, splitter, 50, pulses_fill +1); 
   std::thread out_1 (get, splitter, 50, pulses_read +1, ids.at(0), &cbuf, 500); 
   std::thread out_2 (get, splitter, 50, pulses_read +1, ids.at(1), &cbuf2, 0); 
   std::thread out_3 (get, splitter, 50, pulses_read +1, ids.at(2), &cbuf3, 400); 
  out_1.join();
  out_2.join();
  out_3.join();
  in_1.join();
  //in_2.join();

  std::cout << "Client Buffer3: size=" << cbuf3.size() << std::endl;
  for (unsigned i=0; i < cbuf3.size(); i++) {
    for (unsigned j=0; j < cbuf3.at(i)->size(); j++)
    std::cout << (int)cbuf3.at(i)->at(j) << " ";
  }
  std::cout << std::endl;
  // ------
  
  std::cout << "Client Buffer2: size=" << cbuf2.size() << std::endl;
  for (unsigned i=0; i < cbuf2.size(); i++) {
    for (unsigned j=0; j < cbuf2.at(i)->size(); j++)
    std::cout << (int)cbuf2.at(i)->at(j) << " ";
  }
  std::cout << std::endl;
  // ------

  std::cout << "Client Buffer: size=" << cbuf.size() << std::endl;
  for (unsigned i=0; i < cbuf.size(); i++) {
    for (unsigned j=0; j < cbuf.at(i)->size(); j++)
    std::cout << (int)cbuf.at(i)->at(j) << " ";
  }
  std::cout << std::endl;
  // ------

  std::deque<std::shared_ptr<std::vector<uint8_t>>> buf = splitter->getBuffer();
  std::cout << "Buffer: size=" << buf.size() << std::endl;
  for (unsigned i=0; i < buf.size(); i++) {
    for (unsigned j=0; j < buf.at(i)->size(); j++)
    std::cout << (int)buf.at(i)->at(j) << " ";
  }
  std::cout << std::endl;
  // ------
  std::cout << "Clients status" << std::endl;  
  int client = -1;
  int latency = -1;
  for(int i = 0; i < _nMaxClients; i++) {
    bool is = splitter->SplitterClientGetByIndex(i, &client, &latency);
    if(is)
      std::cout << "id:" << client << "|" << "c:" << latency << " ";
  }
  std::cout << std::endl;
  

  int out = 0;
  std::cin >> out; 
  
  return 0;
}
