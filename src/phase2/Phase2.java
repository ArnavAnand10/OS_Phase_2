/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
// package phase2;

/**
 *
 * @author kiran
 */
import java.io.*;
import java.util.*;

public class Phase2 {
    // Hardware Resources
    static String mainMemory[][] = new String[300][4], instructionRegister[] = new String[4], generalPurposeReg[] = new String[4];
    static String DTApart = "",ErrorMessage = "";
    static int instructionCounter,DTAPointer,TI = 0,PI = 0;
    static int SI, M=0,TTL=0,JID=0,TLL=0,jobStarted = 0,TTC = 0,LLC = 0;
    static int PTR[] = new int [2];
    static boolean  toggle, continueExecution = false;

    static File outputFile = new File("/home/adi/main/OS_Phase_2/src/phase2/output.txt");
    static File inputFile = new File("/home/adi/main/OS_Phase_2/src/phase2/input.txt");

    //======================================
    // brief methods
    //======================================
    // print
    static void print(Object args) {
        System.out.println(args);
    }

    // Getting no of frames from number of words
    static int getFrameNos(int words){
        if(10%words == 0) return words/10;
        else return (words/10)+1;
    }

    // main memory display
    static void dispMainMemory(){
        print("\nThe main memory");
        for(int i=0;i<300;i++){
            System.out.print(i+"- ");
            for(int j = 0;j<4;j++){
                System.out.print(mainMemory[i][j]);
            }print("");
        }
    }

    // Displaying file function
    static void fileDisplay(File fileName) {
        try {
            int ch;
            FileReader toDispFile = new FileReader(fileName);

            while ((ch = toDispFile.read()) != -1) {
                System.out.print((char) ch);
            }
            toDispFile.close();
        } catch (Exception ex) {
            System.out.println("Error in Reading file "+ex);
        }
    }

    // Write the user input on file
    static void fileWrite() {
        String temp = "";
        List<String> userProgram = new ArrayList<String>();

        print("Enter your program below");
        Scanner userInput = new Scanner(System.in);

        while (!temp.equals("$END")) {
            String inputBuff = userInput.nextLine();
            userProgram.add(inputBuff);
            temp = inputBuff;
        }

        try {
            FileWriter fw = new FileWriter("input.txt");

            for(int i = 0; i<userProgram.size();i++){
                fw.append(userProgram.get(i)+"\n");
            }

            fw.close();
        } catch (Exception ex) {
            print("Error writing file "+ex);
        }
    }

    //loading to main memory
    static void loadToMemory1(String data, int frames){
        int stringIndex = 0;
        int VA = 0, RA = 0, CVA = 1;

        // Loading to main memory and mapping page tables
        //------------------------------------------------------------
        for(int i = 0;i<frames;i++){
            // Extracting VA from IR
            try{
                for(int m = 2; m<4;m++) {
                    VA = VA*10 + Integer.parseInt(instructionRegister[m]);
                }
            } catch (NumberFormatException exception){
                if(VA == CVA){
                    VA+=10;
                }
                CVA = VA;
            }

            //Mapping VA to RA
            if(getRA(VA)<0) mapAddress(VA);

            // Getting The Real Address
            RA = getRA(VA);

            // Loading the data to main memory
            for(int l = RA;l<RA+10;l++){
                for(int k=0;k<4;k++){
                    if(stringIndex<data.length()){
                        mainMemory[l][k] = String.valueOf(data.charAt(stringIndex));
                        stringIndex++;
                    } else {
                        k = 4000;
                        l = 20000;
                    }
                }
            }
        }
    }

    //Cleaning Main Memory
    static void cleanMainMemory(){
        for(int i=0;i<300;i++){
            for(int j = 0;j<4;j++){
                mainMemory[i][j] = null;
            }
        }
    }

    // Getting Real Address from passed Virtual Address
    static int getRA(int VA){
        int RA = 0;
        int offset = VA%10;
        int base = VA/10;

        // searching for the VA in page table
        for(int i = PTR[0];i<PTR[1];i++){
            int pageTableBase = 0;

            // Extracts VAs from page table
            for(int j = 0;j<2;j++){
                pageTableBase = pageTableBase*10+Integer.parseInt(mainMemory[i][j]);
            }

            // Matching page table VA to VA which program wants
            if(base == pageTableBase){
                // In case they match we will generate the RA as follow
                // To do that we get base of RA related to VA form page table
                for(int j = 2;j<4;j++){
                    RA = RA*10+Integer.parseInt(mainMemory[i][j]);
                }

                RA = RA*10+offset;
                return RA;
            }
        }

        // Eventually return null in case virtual address doesn't match with any of the VRs in page table
        return -1;
    }

    //======================================
    // Phase Two methods
    //======================================
    // Mapping VR to AR
    static void mapAddress(int VA){
        // Searching for hole where we can place our page
        VA = (VA/10);
        for(int j = 0;j<27;j++){ // we just search in 27 blocks (frames) because last tree ones are just allocated for page table
            try{
                if(mainMemory[j*10][0].equals("arg0"));
            } catch (Exception e){
                // Updating Page Table
                int RA = j;

                // Mapping page to frame
                mainMemory[PTR[1]][0] = String.valueOf(VA/10);
                mainMemory[PTR[1]][1] = String.valueOf(VA%10);
                mainMemory[PTR[1]][2] = String.valueOf(RA/10);
                mainMemory[PTR[1]][3] = String.valueOf(RA%10);

                // Updating PTR 
                PTR[1]++;
                j= 456;
            }
        }
    }

    //Read function
    static void read(){
        //Re_initializing the SI interruption code
        SI = 0;

        //Read next (data) card from input file in memory locations IR [3,4] through IR [3,4] +9
        instructionRegister[3] = "0";
        int memoryAddress = 0;
        for(int i =2;i<4;i++) memoryAddress = memoryAddress*10 + Integer.parseInt(String.valueOf(instructionRegister[i])); //Getting the Address of the main memory to put data card in.
        M = memoryAddress;

        // Catching out of Data Error 
        if(DTApart.length() == 0 || DTAPointer >= DTApart.split("#").length){
            terminate(1);
            return;
        }

        //Loading into the main memory
        //-----------------------------
        String dataCardHolder[] = DTApart.split("#");
        if(DTAPointer<dataCardHolder.length){
            loadToMemory1(dataCardHolder[DTAPointer], 1);
            DTAPointer++;
        }
    }

    //Write function
    static void write(){
        //Write one block (10 words of memory) from memory locations IR [3,4] through IR [3,4] + 9 to output file
        int memoryAddress = 0;

        //Re_initializing the SI interruption code
        SI = 0;

        // Extracting VA from IR
        for(int i =2;i<4;i++) memoryAddress = memoryAddress*10 + Integer.parseInt(String.valueOf(instructionRegister[i]));

        // Getting the RA from given VA
        memoryAddress = getRA(memoryAddress);

        try{
            // Catching Line Limit Exceed
            if(LLC >= TLL){
                terminate(2);
                continueExecution = false;
                return;
            } else {
                String toBeWritten = "";
                FileWriter wf = new FileWriter(outputFile,true);
                PrintWriter printWF = new PrintWriter(wf);
                inputFile.setWritable(true);

                //Going through the main memory
                for (int i = memoryAddress; i < memoryAddress+10; i++) {
                    for (int j = 0; j < 4 && mainMemory[i][j] != null; j++) {
                        toBeWritten = toBeWritten.concat(mainMemory[i][j]);
                    }
                }

                //Writing into the file
                printWF.write(toBeWritten + "\n");

                // Incrementing the LLC
                LLC++;

                // Closing the output file and it writer
                printWF.close();
                wf.close();
            }

        } catch (Exception ex){
            print("Write() cannot write into output file:");
            ex.printStackTrace();
        }
    }

    //Terminate function 
    static void terminate(int code){
        try{
            FileWriter wf = new FileWriter(outputFile,true);
            PrintWriter writer = new PrintWriter(wf);

            // Getting IR contents
            String IR = "";
            try {
                for(int i=0;i<4;i++) {
                    if(instructionRegister[i] != null) {
                        IR = IR.concat(instructionRegister[i]);
                    }
                }
            } catch (Exception e) {
                // Handle exception
            }

            //Let's not let executeUserProgram() to run further
            continueExecution = false;

            // Termination possibilities - MATCHING DESIRED OUTPUT FORMAT
            //-------------------------------------------------------
            if(code == 0){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n NO ERROR\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 1){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n OUT OF DATA\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 2){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n LINE LIMIT EXCEEDED\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 3){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n TIME LIMIT EXCEEDED\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 4){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n OPERATION CODE ERROR\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 5){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n OPERAND ERROR\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 6){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n INVALID PAGE FAULT\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 7){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n TIME LIMIT EXCEEDED And OPERATION CODE ERROR\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            } else if(code == 8){
                ErrorMessage = "JOB ID   :  " + String.valueOf(JID) + "\n TIME LIMIT EXCEEDED And OPERAND ERROR\n" +
                        "IC       :  " + instructionCounter + "\nIR       :  " + IR + "\nTTC      :  " + TTC +
                        "\nLLC      :  " + LLC;
            }

            outputFile.setWritable(true);
            writer.append("\n" + ErrorMessage + "\n\n");
            writer.close();
        } catch (Exception ex) {
            print("Not able to terminate");
            ex.printStackTrace();
        }
    }


    //THE MOS FUNCTION
    static void MOS(){
        // Check for combined errors first
        if (TI == 2 && PI == 1) {
            terminate(7); // TLE + Opcode error
            return;
        } else if (TI == 2 && PI == 2) {
            terminate(8); // TLE + Operand error
            return;
        }
        
        // Then check individual errors
        if(TI == 0){
            if(SI == 1){
                read();
            } else if (SI == 2){
                write();
            } else if (SI == 3){
                terminate(0);
            } else if (PI == 1){
                terminate(4);
            } else if (PI == 2){
                terminate(5);
            } else if (PI == 3){
                terminate(6);
            }
        } else if(TI == 2){
            if(SI == 1){
                terminate(3);
            } else if (SI == 2){
                write();
                terminate(3);
            } else if (SI == 3){
                terminate(0);
            } else if (PI == 1){
                terminate(7);
            } else if (PI == 2){
                terminate(8);
            } else if (PI == 3){
                terminate(3);
            } else {
                terminate(3); // Pure TLE
            }
        }
    }

//execute user program function
static void executeUserProgram(){
    while(continueExecution){
        // Check time limit at the start of each instruction
        if (checkTimeLimit()) {
            MOS();
            break;
        }

        // Fetch instruction
        for(int i = 0; i<4;i++) {
            instructionRegister[i] = mainMemory[instructionCounter][i];
        }
        
        // Check time limit after instruction fetch
        if (checkTimeLimit()) {
            MOS();
            break;
        }

        instructionCounter++;

        //Examining the instruction
        String instruction = "";
        int memoryAddress = 0;
        for (int i = 0; i < 2 && instructionRegister[i] != null; i++) {
            instruction = instruction.concat(instructionRegister[i]);
        }
        
        try {
            for (int i = 2; i < 4 && instructionRegister[i] != null; i++) {
                memoryAddress = (memoryAddress*10) + Integer.parseInt(instructionRegister[i]);
            }
        } catch (Exception e) {
            // Catching Operand Error
            PI = 2;
            TTC++; // Count invalid instructions too
            MOS();
            continueExecution = false;
            continue;
        }

        // Check time limit after instruction decode
        if (checkTimeLimit()) {
            MOS();
            break;
        }

        // Updating memoryAddress with getRA() from VA to RA
        boolean pageFault = false;
        if(getRA(memoryAddress) >= 0){
            memoryAddress = getRA(memoryAddress);
        } else {
            pageFault = true;
        }

        // Executing the instructions - INCREMENT TTC FOR ALL INSTRUCTIONS
        if(instruction.equals("LR")){
            TTC++;
            if(!pageFault){
                try{
                    for(int i = 0; i<4;i++) {
                        generalPurposeReg[i] = mainMemory[memoryAddress][i];
                    }
                } catch (Exception exception){
                    PI = 3;
                    MOS();
                    continueExecution = false;
                }
            } else {
                PI = 3;
                MOS();
                continueExecution = false;
            }
        } else if(instruction.equals("SR")){
            TTC++;
            String strToBeLoaded = "";
            try {
                for(int i = 0; i<4;i++) {
                    if (generalPurposeReg[i] != null) {
                        strToBeLoaded = strToBeLoaded.concat(generalPurposeReg[i]);
                    }
                }
            } catch (Exception e) {
                // Handle exception
            }
            loadToMemory1(strToBeLoaded, 1);
        } else if (instruction.equals("CR")){
            TTC++;
            toggle = false;
            if(!pageFault) {
                try{
                    boolean allMatch = true;
                    for(int i = 0; i<4;i++){
                        if(generalPurposeReg[i] == null || mainMemory[memoryAddress][i] == null || 
                           !generalPurposeReg[i].equals(mainMemory[memoryAddress][i])) {
                            allMatch = false;
                            break;
                        }
                    }
                    toggle = allMatch;
                } catch (Exception exception){
                    PI = 3;
                    MOS();
                    continueExecution = false;
                }
            } else {
                PI = 3;
                MOS();
                continueExecution = false;
            }
        } else if (instruction.equals("BT")){
            TTC++;
            if(toggle) instructionCounter = memoryAddress;
        } else if (instruction.equals("GD")){
            TTC++; // COUNT system calls too
            SI = 1;
            MOS();
        } else if (instruction.equals("PD")) {
            TTC++; // COUNT system calls too
            if(!pageFault){
                SI = 2;
                MOS();
            } else {
                PI = 3;
                MOS();
                continueExecution = false;
                continue;
            }
        } else if (instruction.equals("H")){
            TTC++; // COUNT system calls too
            SI = 3;
            MOS();
            continueExecution = false;
        } else {
            TTC++; // Count invalid instructions
            PI = 1;
            MOS();
            continueExecution = false;
        }

        // Final time limit check after instruction execution
        if (checkTimeLimit()) {
            MOS();
            break;
        }
    }
}

// Check for Time Limit Exceeded - FIXED LOGIC
static boolean checkTimeLimit() {
    // Only check if TTL > 0 and TTC has reached or exceeded TTL
    if (TTL > 0 && TTC > TTL) {  // Changed from >= to > to match desired behavior
        TI = 2;
        return true;
    }
    return false;
}


// Start Execution function - RESET COUNTERS PROPERLY
static void startExecution(){
    instructionCounter = 0;
    // Reset TTC and LLC for each job
    TTC = 0;
    LLC = 0;
    TI = 0;
    PI = 0;
    SI = 0;
    //calling executeUserProgram
    executeUserProgram();
}


    //LOAD function
static void load(){
    try{
        String loadBuffer = "";
        FileReader fileReader = new FileReader(inputFile);
        BufferedReader reader = new BufferedReader(fileReader);

        // Reading the lines of the input file.
        loadBuffer = reader.readLine();
        while(loadBuffer != null){
            // Check if line is long enough before substring
            if(loadBuffer.length() >= 4 && loadBuffer.substring(0, 4).equals("$AMJ")){
                //Doing all needed for Start part
                TTL = 0;
                TLL = 0;
                JID = 0;
                M = 0;

                // Reinitialized Counters
                TTC = 0;
                LLC = 0;

                // Interruption codes initializing
                SI = 0;
                PI = 0;
                TI = 0;

                // Cleaning Memory at the first of the job
                cleanMainMemory();

                // Cleaning IR 
                for (int i = 0; i < 4; i++) {
                    instructionRegister[i] = null;
                }

                // Initializing the PCB and PTR
                for(int i = 270,j = 27;i<273;i++,j++){
                    mainMemory[i][0] = String.valueOf(j/10);
                    mainMemory[i][1] = String.valueOf(j%10);
                    mainMemory[i][2] = String.valueOf(j/10);
                    mainMemory[i][3] = String.valueOf(j%10);
                }

                // Initializing the PTR
                PTR[0] = 270;
                PTR[1] = 273;

                //Getting the TTL, JID, TLL values
                for(int i=4;i<8;i++) JID = JID*10 + Integer.parseInt(String.valueOf(loadBuffer.charAt(i)));
                for(int i=8;i<12;i++) TTL = TTL*10 + Integer.parseInt(String.valueOf(loadBuffer.charAt(i)));
                for(int i=12;i<16;i++) TLL = TLL*10 + Integer.parseInt(String.valueOf(loadBuffer.charAt(i)));

                //Loading the Control card to main memory
                loadBuffer = reader.readLine();
                String controlsToBeLoaded = "";

                while(loadBuffer != null && (loadBuffer.length() < 4 || !loadBuffer.substring(0, 4).equals("$DTA"))){
                    controlsToBeLoaded = controlsToBeLoaded.concat(loadBuffer);
                    loadBuffer = reader.readLine();
                    if (loadBuffer == null) break;
                }

                // Loading the control cards into main memory
                loadToMemory1(controlsToBeLoaded, getFrameNos(controlsToBeLoaded.length()));

                //After loading the control cart to main memory we need to take care of the data part.
                DTApart = "";
                DTAPointer = 0;

                if (loadBuffer != null && loadBuffer.length() >= 4 && loadBuffer.substring(0, 4).equals("$DTA")) {
                    loadBuffer = reader.readLine();
                    while(loadBuffer != null && (loadBuffer.length() < 4 || !loadBuffer.substring(0, 4).equals("$END"))){
                        DTApart = DTApart.concat(loadBuffer + "#");
                        loadBuffer = reader.readLine();
                    }
                }

                //calling for startExecution()
                continueExecution = true;
                startExecution();
            }
            loadBuffer = reader.readLine();
        }
        reader.close();
    } catch (Exception exception) {
        print("load(): cannot read the input file:");
        exception.printStackTrace();
    }
}

    public static void main(String[] args) {
        // Clear output file before starting
        try {
            FileWriter fw = new FileWriter(outputFile);
            fw.write("");
            fw.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        fileDisplay(inputFile);
        load();
        fileDisplay(outputFile);
    }
}