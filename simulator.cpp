#include <iostream>     // สำหรับ input/output มาตรฐาน (cout, cin)
#include <fstream>      // สำหรับอ่านไฟล์ (ifstream)
#include <vector>       // สำหรับใช้ vector แทน memory
#include <iomanip>      // สำหรับการปริ๊นท์ฟอร์แมต (แต่ในที่นี้อาจไม่มีใช้มากนัก)
using namespace std;

struct State {
    int pc = 0;                  // Program Counter: ตำแหน่งคำสั่งปัจจุบันใน memory
    vector<int> memory;          // memory: เก็บคำสั่งและข้อมูล
    int numMemory = 0;           // บอกจำนวนช่องที่มีค่าจากไฟล์ (ช่องอื่นๆ เป็น 0)
    int reg[8] = {0};            // registers: SMC มี 8 register reg[0]~reg[7]
};

void printState(const State &s) {
    cout << "@@@\n";
    cout << "state:\n";
    cout << "\tpc " << s.pc << '\n';
    cout << "\tmemory:\n";
    for(int i=0; i<s.numMemory; ++i)
        cout << "\t\tmem[ " << i << " ] " << s.memory[i] << '\n';
    cout << "\tregisters:\n";
    for(int i=0; i<8; ++i)
        cout << "\t\treg[ " << i << " ] " << s.reg[i] << '\n';
    cout << "end state\n";
}

int opcode(int instr) { return (instr>>22)&0x7; } //opcode: bits 24-22
int regA(int instr) { return (instr>>19)&0x7; } //regA, regB: ระบุ register a, b
int regB(int instr) { return (instr>>16)&0x7; }
int destReg(int instr) { return instr&0x7; } //destReg: ระบุ register เป้าหมาย (R-type)
int offset(int instr) { //offset: ใช้กับคำสั่ง I-type (เช่น lw, sw, beq) และต้อง sign-extend จาก 16 bit
    int off = instr&0xFFFF;
    if(off & 0x8000) off |= 0xFFFF0000; // sign-extend 16 to 32 bit
    return off;
}

void runSimulator(State &s) {
    int executed = 0;
    while (true) {
        printState(s);
        int instr = s.memory[s.pc];
        int op = opcode(instr);
        int a = regA(instr);
        int b = regB(instr);
        int next_pc = s.pc + 1;

        if(op==0) { // add
            s.reg[destReg(instr)] = s.reg[a] + s.reg[b];
        }else if(op==1) { // nand
            s.reg[destReg(instr)] = ~(s.reg[a] & s.reg[b]);
        }else if(op==2) { // lw
            int addr = s.reg[a]+offset(instr);
            s.reg[b] = s.memory[addr];
        }else if(op==3) { // sw
            int addr = s.reg[a]+offset(instr);
            s.memory[addr] = s.reg[b];
        }else if(op==4) { // beq
            if(s.reg[a] == s.reg[b]) 
                next_pc = next_pc + offset(instr);
        }else if(op==5) { // jalr
            s.reg[b] = next_pc;
            next_pc = s.reg[a];
        }else if(op==6) { // halt
            executed++;
            s.pc = next_pc;
            cout << "machine halted\n";
            cout << "total of " << executed << " instructions executed\n";
            cout << "final state of machine:\n";
            printState(s);
            break;
        }else if(op==7) {
            // noop
        }else{
            cout << "Unknown opcode!\n";
            break;
        }
        executed++;
        s.pc = next_pc;
        s.reg[0] = 0; // always
    }
}

// โหลดไฟล์ machine code แล้วรัน
int main(int argc, char** argv){
    if(argc!=2){
        cout << "Usage: ./simulator filename.txt\n";
        return 1;
    }
    State s;
    ifstream in(argv[1]);
    int v;
    while(in >> v) s.memory.push_back(v);
    s.numMemory = (int)s.memory.size();
    // รองรับจำนวน memory index > numMemory ตอน sw
    if(s.memory.size() < 32768)
        s.memory.resize(32768, 0); // ให้ sw/lw ทำงานได้กับ address ที่ใหญ่
    runSimulator(s);
    return 0;
}
