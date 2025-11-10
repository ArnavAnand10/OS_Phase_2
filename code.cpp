#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
using namespace std;
class PCB
{
public:
 int jobID, TTL, TLL, TTC, LLC;
};
class Osphase2
{
private:
 fstream infile;
 ofstream outfile;
 PCB pcb;
 int SI, PI, TI;
 int PTR;
 int visited[30];
 bool mos;
 int RA, VA;
 bool Terminate;
 int PTE;
 int page_fault = 0;
 int pageNo;
 int PageTable_ptr;
 // memory and registers
 char M[300][4];
 char IR[4];
 char R[4];
 char buffer[40];
 int IC;
 bool C;
 void BUFFER(){
 fill(buffer, buffer + sizeof(buffer), '\0');
 }
 void INIT()
 {
 BUFFER();
 fill(&M[0][0], &M[0][0] + sizeof(M), '\0');
 for (int i = 0; i < 4; i++){ 
 IR[i] = ' ';
 R[i] = ' ';
 }
 C = true;
 IC = 0;
 SI = PI = TI = 0;
 pcb.jobID = pcb.TLL = pcb.TTL = pcb.TTC = pcb.LLC = 0;
 PTR = PTE = pageNo = -1;
 for (int i = 0; i < 30; i++)
 {
 visited[i] = 0;
 }
 PageTable_ptr = 0;
 Terminate = false;
 }
 int ALLOCATE(){
 int pageNo;
 bool check = true;
 while (check)
 {
 pageNo = (rand() % 30);
 if (visited[pageNo] == 0)
 {
 visited[pageNo] = 1;
 check = false;
 }
 }
 return pageNo;
 }
 void MOS(){
 if (TI == 0 && SI == 1)
 {
 READ();
 }
 else if (TI == 0 && SI == 2)
 {
 WRITE();
 }
 else if (TI == 0 && SI == 3)
 {
 TERMINATE(0);
 }
 else if (TI == 2 && SI == 1)
 {
 TERMINATE(3);
 }
 else if (TI == 2 && SI == 2)
 {
 WRITE();
 TERMINATE(3);
 }
 else if (TI == 2 && SI == 3)
 {
 TERMINATE(0);
 }
 else if (TI == 0 && PI == 1)
 {
 TERMINATE(4);
 }
 else if (TI == 0 && PI == 2)
 {
 TERMINATE(5);
 }
 else if (TI == 0 && PI == 3)
 {
 if (page_fault == 1)
 {
 cout << " Page Fault: ";
 pageNo = ALLOCATE();
 M[PTE][2] = (pageNo / 10) + '0';
 M[PTE][3] = (pageNo % 10) + '0';
 PageTable_ptr++;
 PI = 0;
 cout << "Allocated Page Number: " << pageNo << "\n";
 }
 else
 {
 TERMINATE(6);
 }
 }
 else if (TI == 2 && PI == 1)
 {
 TERMINATE(7); // Time + Operation code error
 }
 else if (TI == 2 && PI == 2)
 {
 TERMINATE(8); // Time + Operand error
 }
 else if (TI == 2 && PI == 3)
 {
 TERMINATE(3);
 }
 else
 {
 TERMINATE(3);
 }
 }
 void READ(){
 cout << "\nRead function called\n";
 string data;
 getline(infile, data);
 if (data[0] == '$' && data[1] == 'E' && data[2] == 'N' && data[3] == 'D')
 {
 TERMINATE(1);
 return;
 }
 int len = data.size();
 for (int i = 0; i < len; i++){
 buffer[i] = data[i];
 }
 int bufferIndex = 0, mem_ptr = RA, end = RA + 10;
 while (bufferIndex < 40 && buffer[bufferIndex] != '\0' && mem_ptr < end)
 {
 for (int i = 0; i < 4; i++)
 {
 M[mem_ptr][i] = buffer[bufferIndex];
 bufferIndex++;
 }
 mem_ptr++;
 }
 BUFFER();
 SI = 0;
 }
 void WRITE(){
 cout << "\nWrite function called\n";
 pcb.LLC++;
 if (pcb.LLC > pcb.TLL)
 {
 pcb.LLC--;
 TERMINATE(2);
 return;
 }
 outfile.open("output.txt", ios::app);
 string output;
 if (RA != -1)
 {
 for (int i = RA; i < RA + 10; i++)
 {
 for (int j = 0; j < 4; j++)
 {
 if (M[i][j] == '\0')
 {
 M[i][j] = ' ';
 }
 output += M[i][j];
 }
 }
 outfile << output << "\n";
 }
 SI = 0;
 outfile << "\n";
 outfile.close();
 }
 void TERMINATE(int EM)
 {
 Terminate = true;
 outfile.open("output.txt", ios::app);
 outfile << "JOB ID   :  " << pcb.jobID << "\n";
 switch (EM)
 {
 case 0:
 outfile << " NO ERROR\n";
 break;
 case 1:
 outfile << " OUT OF DATA\n";
 break;
 case 2:
 outfile << " LINE LIMIT EXCEEDED\n";
 break;
 case 3:
 outfile << " TIME LIMIT EXCEEDED\n";
 break;
 case 4:
 outfile << " OPERATION CODE ERROR\n";
 break;
 case 5:
 outfile << " OPERAND ERROR\n";
 break;
 case 6:
 outfile << " INVALID PAGE FAULT\n";
 break;
 case 7:
 outfile << " TIME LIMIT EXCEEDED  And OPERATION CODE ERROR\n";
 break;
 case 8:
 outfile << " TIME LIMIT EXCEEDED And OPERAND ERROR\n";
 break;
 }
 outfile << "IC       :  " << IC << "\n";
 outfile << "IR       :  ";
 for (int i = 0; i < 4; i++)
 {
 outfile << IR[i];
 }
 outfile << "\n";
 outfile << "TTC      :  " << pcb.TTC << "\n";
 outfile << "LLC      :  " << pcb.LLC << "\n";
 outfile << "\n\n";
 SI = 0;
 PI = 0;
 TI = 0;
 outfile.close();
 }
 void LOAD(){
 if (infile.is_open())
 {
 string s;
 while (getline(infile, s))
 {
 if (s[0] == '$' && s[1] == 'A' && s[2] == 'M' && s[3] == 'J')
 {
 INIT();
 cout << "\nNew Job started\n";
 pcb.jobID = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
 pcb.TTL = (s[8] - '0') * 1000 + (s[9] - '0') * 100 + (s[10] - '0') * 10 + (s[11] - '0');
 pcb.TLL = (s[12] - '0') * 1000 + (s[13] - '0') * 100 + (s[14] - '0') * 10 + (s[15] - '0');

 PTR = ALLOCATE() * 10;
 for (int i = PTR; i < PTR + 10; i++)
 {
 for (int j = 0; j < 4; j++)
 {
 M[i][j] = '*';
 }
 }
 cout << "\nAllocated Page is for Page Table: " << PTR / 10 << "\n";
 cout << "jobID: " << pcb.jobID << "\nTTL: " << pcb.TTL << "\nTLL: " << pcb.TLL << "\n";
 }
 else if (s[0] == '$' && s[1] == 'D' && s[2] == 'T' && s[3] == 'A')
 {
 cout << "Data card loading\n";
 BUFFER();
 STARTEXECUTION();
 }
 else if (s[0] == '$' && s[1] == 'E' && s[2] == 'N' && s[3] == 'D')
 {
 cout << "\nEND of Job\n";
 }
 else
 {
 BUFFER();
 pageNo = ALLOCATE();
 M[PTR + PageTable_ptr][2] = (pageNo / 10) + '0';
 M[PTR + PageTable_ptr][3] = (pageNo % 10) + '0';
 PageTable_ptr++;
 cout << "Program Card loading\n";
 cout << "Allocated page no. for program card =" << pageNo << "\n";
 int length = s.size();
 for (int i = 0; i < length; i++)
 {
 buffer[i] = s[i];
 }
 int bufferIndex = 0;
 int mem_loc = pageNo * 10;
 int end = mem_loc + 10;
 while (bufferIndex < 40 && buffer[bufferIndex] != '\0' && mem_loc < end)
 {
 for (int j = 0; j < 4; j++)
 {
 if (buffer[bufferIndex] == 'H')
 {
 M[mem_loc][j] = 'H';
 bufferIndex++;
 break;
 }
 M[mem_loc][j] = buffer[bufferIndex];
 bufferIndex++;
 }
 mem_loc++;
 }
 }
 }
 infile.close();
 }
 }
 int ADDRESSMAP(int VA)
 {
 if (0 <= VA && VA < 100){
 PTE = PTR + (VA / 10);
 if (M[PTE][2] == '*')
 {
 PI = 3;
 cout << "\nPage fault occured!\n";
 return -1;
 }else {
 string p;
 p = M[PTE][2];
 p += M[PTE][3];
 int pageNo = stoi(p);
 RA = pageNo * 10 + (VA % 10);
 cout << "\nReturned Real Address (RA) = " << RA;
 return RA;
 }
 }
 else
 {
 PI = 2;
 cout << "\nOperand error called";
 return -1;
 }
 }
 void STARTEXECUTION()
 {
 IC = 0;
 EXECUTEUSERPROGRAM();
 }
 void EXECUTEUSERPROGRAM()
 {
 while (!Terminate)
 {
 mos = false;
 
 RA = ADDRESSMAP(IC);
 if (PI != 0)
 {
 MOS();
 return;
 }
 
 for (int i = 0; i < 4; i++){
 IR[i] = M[RA][i];
 }
 
 IC++;
 
 string op;
 op += IR[2];
 op += IR[3];
 cout << "\nInstruction register has = " << IR[0] << IR[1] << IR[2] << IR[3];
 
 // Check for invalid opcode first (don't increment TTC)
 if (!((IR[0] == 'G' && IR[1] == 'D') || 
       (IR[0] == 'P' && IR[1] == 'D') ||
       (IR[0] == 'L' && IR[1] == 'R') ||
       (IR[0] == 'S' && IR[1] == 'R') ||
       (IR[0] == 'C' && IR[1] == 'R') ||
       (IR[0] == 'B' && IR[1] == 'T') ||
       (IR[0] == 'H' && IR[1] == '\0')))
 {
 PI = 1;
 mos = true;
 }
 // Check for operand errors (don't increment TTC)
 else if ((IR[0] == 'G' && IR[1] == 'D') || 
          (IR[0] == 'P' && IR[1] == 'D') ||
          (IR[0] == 'L' && IR[1] == 'R') ||
          (IR[0] == 'S' && IR[1] == 'R') ||
          (IR[0] == 'C' && IR[1] == 'R') ||
          (IR[0] == 'B' && IR[1] == 'T'))
 {
 if (!isdigit(IR[2]) || !isdigit(IR[3]))
 {
 PI = 2;
 mos = true;
 }
 }
 
 // If no PI error, execute instruction
 if (PI == 0)
 {
 // GD - Get Data
 if (IR[0] == 'G' && IR[1] == 'D')
 {
 SIMULATION();
 if (TI != 2) {
 page_fault = 1;
 VA = stoi(op);
 cout << "\nvirtual address= " << VA;
 RA = ADDRESSMAP(VA);
 if (PI == 0) {
 SI = 1;
 }
 }
 mos = true;
 }
 // PD - Print Data
 else if (IR[0] == 'P' && IR[1] == 'D')
 {
 SIMULATION();
 if (TI != 2) {
 page_fault = 0;
 VA = stoi(op);
 RA = ADDRESSMAP(VA);
 if (PI == 0) {
 SI = 2;
 }
 }
 mos = true;
 }
 else if (IR[0] == 'H' && IR[1] == '\0')
 {
 SIMULATION();
 SI = 3;
 mos = true;
 }
 // LR - LOAD DATA
 else if (IR[0] == 'L' && IR[1] == 'R')
 {
 SIMULATION();
 if (TI != 2) {
 page_fault = 0;
 VA = stoi(op);
 RA = ADDRESSMAP(VA);
 if (PI == 0) {
 for (int i = 0; i < 4; i++)
 {
 R[i] = M[RA][i];
 }
 }
 else {
 mos = true;
 }
 }
 else {
 mos = true;
 }
 }
 // SR - STORE
 else if (IR[0] == 'S' && IR[1] == 'R')
 {
 SIMULATION();
 if (TI != 2) {
 page_fault = 1;
 VA = stoi(op);
 RA = ADDRESSMAP(VA);
 if (PI == 0) {
 for (int i = 0; i < 4; i++){
 M[RA][i] = R[i];
 }
 }
 else {
 mos = true;
 }
 }
 else {
 mos = true;
 }
 }
 // CR
 else if (IR[0] == 'C' && IR[1] == 'R')
 {
 SIMULATION();
 if (TI != 2) {
 page_fault = 0;
 VA = stoi(op);
 RA = ADDRESSMAP(VA);
 if (PI == 0) {
 string s1, s2;
 for (int i = 0; i < 4; i++)
 {
 s1 += M[RA][i];
 s2 += R[i];
 }
 if (s1 == s2)
 {
 C = true;
 }
 else
 {
 C = false;
 }
 }
 else {
 mos = true;
 }
 }
 else {
 mos = true;
 }
 }
 // BT
 else if (IR[0] == 'B' && IR[1] == 'T')
 {
 SIMULATION();
 if (TI != 2) {
 if (C)
 {
 string j;
 j += IR[2];
 j += IR[3];
 IC = stoi(j);
 }
 }
 else {
 mos = true;
 }
 }
 }
 
 if (mos)
 {
 cout << "\nMOS called for : " << IR[0] << IR[1] << IR[2] << IR[3];
 MOS();
 }
 }
 }
 void SIMULATION(){
 // Increment TTC by 1 for each instruction executed
 pcb.TTC += 1;
 
 cout << "\nTTC= " << pcb.TTC;
 if (pcb.TTC >= pcb.TTL)
 {
 TI = 2;
 cout << "\nTime limit exceeded";
 cout << "\nTI = " << TI;
 cout << "\nSI = " << SI;
 cout << "\nPI = " << PI;
 }
}
public:
 Osphase2()
 {
 infile.open("input.txt", ios::in);
 INIT();
 LOAD();
 }
};
int main()
{
 Osphase2 os;
 return 0;
}