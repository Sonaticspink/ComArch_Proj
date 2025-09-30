#include <iostream>     // สำหรับ input/output มาตรฐาน (cout, cin)
#include <fstream>      // สำหรับอ่านไฟล์ (ifstream)
#include <vector>       // สำหรับใช้ vector แทน memory
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

// โหลดไฟล์ machine code แล้วรัน
int main(int argc, char** argv){
}