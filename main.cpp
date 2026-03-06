#include <iostream>
#include <string>

// ดึงไฟล์ของเพื่อนร่วมทีมเข้ามาใช้งาน 
// (สมมติว่าเซฟคลาสของเพื่อนไว้ในไฟล์ .h เหล่านี้แล้ว)
#include "CacheSimulator.h" 
#include "TraceParser.h"

using namespace std;

int main() {
    int cacheSize, blockSize, ways, policy;
    string filename;

    cout << "========================================\n";
    cout << "      CPU Cache Mapping Simulator       \n";
    cout << "========================================\n\n";

    // 1. ส่วนรับค่าพารามิเตอร์ (User Interface)
    cout << "--- Cache Configuration ---\n";
    
    cout << "Enter Cache Size (Bytes) [e.g., 1024]: ";
    cin >> cacheSize;
    
    cout << "Enter Block Size (Bytes) [e.g., 16, 32]: ";
    cin >> blockSize;
    
    cout << "Enter Associativity (Ways) [1 = Direct, 2 = 2-Way...]: ";
    cin >> ways;
    
    cout << "Select Replacement Policy (0 = LRU, 1 = FIFO): ";
    cin >> policy;
    
    cout << "Enter Trace Filename [e.g., test_trace.txt]: ";
    cin >> filename;

    cout << "\n========================================\n";
    cout << "Initializing System...\n";

    // 2. สร้าง Object จากคลาสของเพื่อน
    // ส่งค่าที่รับจากคีย์บอร์ดเข้าไปตั้งค่า Cache
    CacheSimulator myCache(cacheSize, blockSize, ways, policy);
    
    // ส่งชื่อไฟล์เข้าไปให้ TraceParser เริ่มอ่านข้อมูล
    TraceParser myParser(filename);

    // เช็คว่าไฟล์เปิดสำเร็จไหม ถ้าไม่สำเร็จให้จบโปรแกรมเลย
    if (myParser.isReady() == false) {
        cout << "\n[System Error] Terminating program due to file error.\n";
        return 1; 
    }

    cout << "\nSimulation Started. Processing instructions...\n";
    cout << "----------------------------------------\n";

    int instructionCount = 0;

    // 3. จุดเชื่อมต่อระบบ (System Integration)
    // วนลูปอ่านไฟล์ทีละบรรทัด แล้วโยน Address เข้าไปใน Cache
    while (true) {
        // ดึงข้อมูล 1 คำสั่งจากโค้ดของเพื่อน (Data Manager)
        MemoryAccess mem = myParser.getNextAccess();
        
        // ถ้า valid เป็น false แปลว่าอ่านจบไฟล์แล้ว ให้หยุดลูป
        if (mem.valid == false) {
            break; 
        }

        // นำ Address ที่ได้ โยนเข้าไปในโค้ดจำลอง Cache ของเพื่อน (Core Architect)
        myCache.accessMemory(mem.address);
        
        instructionCount++;
        
        // ทำระบบโหลดดิ้ง (Loading) เล็กๆ เพื่อไม่ให้โปรแกรมดูค้างเวลาไฟล์ใหญ่ๆ
        if (instructionCount % 100000 == 0) {
            cout << "Processed " << instructionCount << " instructions...\n";
        }
    }

    cout << "----------------------------------------\n";
    cout << "Simulation Completed!\n";
    cout << "Total Instructions Processed: " << instructionCount << " lines\n\n";

    // 4. แสดงผลลัพธ์ (Output)
    myCache.printStats();

    cout << "========================================\n";
    cout << "Press Enter to exit...";
    cin.ignore(); // เคลียร์บัฟเฟอร์
    cin.get();    // รอให้ผู้ใช้กด Enter ค่อยปิดหน้าต่าง
    
    return 0;
}