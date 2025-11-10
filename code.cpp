    #include <algorithm>
        #include <cctype>
        #include <cstddef>
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
    fill(&M[0][0], &M[0][0] + sizeof(M), ' ');
    for (int i = 0; i < 4; i++){ // Initialize IR and R
    IR[i] = ' ';
    R[i] = ' ';
    }
    C = false;
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
    pageNo = (rand() % 30); // 0 - 29
    if (visited[pageNo] == 0)
    {
    visited[pageNo] = 1; // mark as visited
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
    if (PI == 0)
    {
    WRITE();
    }
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
    TI = 0;
    SI = 0;
    cout << "Allocated Page Number: " << pageNo << "\n";
    }
    else
    {
    TERMINATE(6);
    }
    }
    else if (TI == 2 && PI == 1)
    {
    TERMINATE(3);
    }
    else if (TI == 2 && PI == 2)
    {
    TERMINATE(3);
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
    data.erase(remove(data.begin(), data.end(), '\r'), data.end());
    if (data[0] == '$' && data[1] == 'E' && data[2] == 'N' && data[3] == 'D')
    {
    TERMINATE(1);
    return;
    }
    int len = data.size();
    for (int i = 0; i < len && i < 40; i++){ // fill the buffer with data
    buffer[i] = data[i];
    }
    int bufferIndex = 0, mem_ptr = RA, end = RA + 10;
    while (bufferIndex < 40 && buffer[bufferIndex] != '\0' && mem_ptr < end)
    {
    for (int i = 0; i < 4; i++)
    {
    M[mem_ptr][i] = buffer[bufferIndex];
    bufferIndex++;
    if (bufferIndex >= 40 || buffer[bufferIndex] == '\0')
        break;
    }
    mem_ptr++;
    }
    BUFFER();
    SI = 0;
    }
    void WRITE(){
    cout << "\nWrite function called\n";
    if (pcb.LLC + 1 > pcb.TLL)
    {
    TERMINATE(2);
    return;
    }

    outfile.open("output.txt", ios::app);
    string output = "";
    if (RA != -1)
    {
    for (int i = RA; i < RA + 10; i++)
    {
    for (int j = 0; j < 4; j++)
    {
    char ch = M[i][j];
    if (ch == '\0')
    {
    ch = ' ';
    }
    output += ch;
    }
    }
    }
    while (output.size() < 40)
    {
    output += ' ';
    }
    outfile << output << "\n";
    outfile.close();

    pcb.LLC++;
    SI = 0;
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
    if (TI == 2 && PI == 1){
    outfile << " TIME LIMIT EXCEEDED  And OPERATION CODE ERROR\n";
    }
    else if (TI == 2 && PI == 2){
    outfile << " TIME LIMIT EXCEEDED And OPERAND ERROR\n";
    }
    else {
    outfile << " TIME LIMIT EXCEEDED\n";
    }
    break;
    case 4:
    outfile << " OPERATION CODE ERROR\n";
    break;
    case 5:
    outfile << " OPERAND ERROR\n";
    break;
    case 6:
    outfile << " INVALID PAGE FAULT\n";
    }
    outfile << "IC       :  " << IC << "\n";
    outfile << "IR       :  ";
    for (int i = 0; i < 4; i++)
    {
    char ch = IR[i];
    if (ch == '\0')
        ch = ' ';
    outfile << ch;
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
    pcb.jobID = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6]
    - '0') * 10 + (s[7] - '0');
    pcb.TTL = (s[8] - '0') * 1000 + (s[9] - '0') * 100 + (s[10] -
    '0') * 10 + (s[11] - '0');
    pcb.TLL = (s[12] - '0') * 1000 + (s[13] - '0') * 100 + (s[14]
    - '0') * 10 + (s[15] - '0');

    PTR = ALLOCATE() * 10;
    for (int i = PTR; i < PTR + 10; i++)
    {
    for (int j = 0; j < 4; j++)
    {
    M[i][j] = '*';
    }
    }
    cout << "\nAllocated Page is for Page Table: " << PTR / 10 <<
    "\n";
    cout << "jobID: " << pcb.jobID << "\nTTL: " << pcb.TTL <<
    "\nTLL: " << pcb.TLL << "\n";
    }
    else if (s[0] == '$' && s[1] == 'D' && s[2] == 'T' && s[3] ==
    'A')
    {
    cout << "Data card loading\n";
    BUFFER();
    STARTEXECUTION();
    }
    else if (s[0] == '$' && s[1] == 'E' && s[2] == 'N' && s[3] ==
    'D')
    {
    cout << "\nEND of Job\n";
    cout << "Memory Contents:\n";
    cout << "-------------------\n";
    int j;
    for (int i = 0; i < 300; i++)
    {
    if (i == PTR) {
    cout << "\n--- Page Table Start ---\n";
    }
    if (i == PTR + 10) {
    cout << "--- Page Table End ---\n\n";
    }
    if ((i) % 10 == 0)
    {
    cout << " ________________" << endl;
    }
    cout << "| M[" << setw(2) << setfill('0') << i << "]: ";

    for (j = 0; j < 4; j++)
    {
    cout << M[i][j];
    }
    cout << "\t|" << endl;
    if ((i + 1) % 10 == 0)
    {
    cout << "|_______________|\n"
    << endl;
    }
    }
    cout << "|-------------|" << endl;
    }
    else
    {
    BUFFER();
    // Get Frame for Program Page
    pageNo = ALLOCATE();
    M[PTR + PageTable_ptr][2] = (pageNo / 10) + '0';
    M[PTR + PageTable_ptr][3] = (pageNo % 10) + '0';
    PageTable_ptr++;
    cout << "Program Card loading\n";
    cout << "Allocated page no. for program card =" << pageNo <<
    "\n";
    int length = s.size();
    for (int i = 0; i < length; i++)
    {
    buffer[i] = s[i];
    }
    int bufferIndex = 0;
    IC = pageNo * 10;
    int end = IC + 10;
    while (bufferIndex < 40 && buffer[bufferIndex] != '\0' && IC
    < end)
    {
    for (int j = 0; j < 4; j++)
    {
    if (buffer[bufferIndex] == 'H')
    {
    M[IC][j] = 'H';
    bufferIndex++;
    break;
    }
    M[IC][j] = buffer[bufferIndex];
    bufferIndex++;
    }
    IC++;
    }
    }
    }
    infile.close();
    }
    }
    int ADDRESSMAP(int VA)
    {
    if (0 <= VA && VA < 100){ // check validity of virtual address
    PTE = PTR + (VA / 10);
    if (M[PTE][2] == '*')
    {
    PI = 3;
    cout << "\nPage fault occured!\n";
    MOS();
    return -1;
    }

    string p;
    p = M[PTE][2];
    p += M[PTE][3];
    int resolvedPage = stoi(p);
    RA = resolvedPage * 10 + (VA % 10);
    cout << "\nReturned Real Address (RA) = " << RA;
    return RA;
    }

    PI = 2;
    cout << "\nOperand error called";
    MOS();
    return -1;
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
    if (RA == -1)
    {
    mos = false;
    continue;
    }
    for (int i = 0; i < 4; i++){
    IR[i] = M[RA][i];
    }
    
    string op;
    op += IR[2];
    op += IR[3];
    cout << "\nInstruction register has = " << IR[0] << IR[1] << IR[2] <<
    IR[3];
    // GD - Get Data
    if (IR[0] == 'G' && IR[1] == 'D')
    {
    IC++;
    page_fault = 1;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
    VA = stoi(op);
    cout << "\nvirtual address= " << VA;
    RA = ADDRESSMAP(VA);
    if (PI == 0)
    {
    SI = 1;
    }
    }
    SIMULATION();
    if (SI != 0 || PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    // PD - Print Data
    else if (IR[0] == 'P' && IR[1] == 'D')
    {
    IC++;
    page_fault = 0;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
    VA = stoi(op);
    RA = ADDRESSMAP(VA);
    if (PI == 0)
    {
    SI = 2;
    }
    }
    SIMULATION();
    if (SI != 0 || PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    else if (IR[0] == 'H' && IR[1] == ' ')
    {
    IC++;
    SI = 3;
    SIMULATION();
    mos = true;
    Terminate = true;
    }
    // LR - LOAD DATA
    else if (IR[0] == 'L' && IR[1] == 'R')
    {
    IC++;
    page_fault = 0;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
    VA = stoi(op);
    RA = ADDRESSMAP(VA);
    if (PI == 0) {
    for (int i = 0; i < 4; i++)
    {
    R[i] = M[RA][i];
    }
    }
    }
    SIMULATION();
    if (PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    // SR - STORE
    else if (IR[0] == 'S' && IR[1] == 'R')
    {
    IC++;
    page_fault = 1;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
    VA = stoi(op);
    RA = ADDRESSMAP(VA);
    if (PI == 0) {
    for (int i = 0; i < 4; i++){
    M[RA][i] = R[i];
    }
    }
    }
    SIMULATION();
    if (PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    // CR
    else if (IR[0] == 'C' && IR[1] == 'R')
    {
    IC++;
    page_fault = 0;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
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
    }
    SIMULATION();
    if (PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    // BT
    else if (IR[0] == 'B' && IR[1] == 'T')
    {
    IC++;
    page_fault = 0;
    if (!isdigit(IR[2]) || !isdigit(IR[3]))
    {
    PI = 2;
    }
    else
    {
    if (C)
    {
    string j;
    j += IR[2];
    j += IR[3];
    IC = stoi(j);
    }
    }
    SIMULATION();
    if (PI != 0 || TI != 0)
    {
    mos = true;
    }
    }
    else
    {
    IC++;
    PI = 1;
    SI = 0;
    SIMULATION();
    mos = true;
    }
    if (mos)
    {
    cout << "\nMOS called for : " << IR[0] << IR[1] << IR[2] <<
    IR[3];
    MOS();
    }
    }
    }
    void SIMULATION(){
    if (IR[0] == 'G' && IR[1] == 'D')
    {
    pcb.TTC += 2;
    }
    else if (IR[0] == 'P' && IR[1] == 'D')
    {
    pcb.TTC += 1;
    }
    else if (IR[0] == 'H')
    {
    pcb.TTC += 1;
    }
    else if (IR[0] == 'L' && IR[1] == 'R')
    {
    pcb.TTC += 1;
    }
    else if (IR[0] == 'S' && IR[1] == 'R')
    {
    pcb.TTC += 2;
    }
    else if (IR[0] == 'C' && IR[1] == 'R')
    {
    pcb.TTC += 1;
    }
    else if (IR[0] == 'B' && IR[1] == 'T')
    {
    pcb.TTC += 1;
    }
    else
    {
    pcb.TTC += 1;
    }
    cout << "\nTTC= " << pcb.TTC;
    
    // Check time limit AFTER incrementing
    if (pcb.TTC > pcb.TTL)
    {
    TI = 2;
    cout << "\nTime limit exceeded";
    cout << "\nTI = " << TI;
    cout << "\nSI = " << SI;
    cout << "\nPI = " << PI;
    mos = true;
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
