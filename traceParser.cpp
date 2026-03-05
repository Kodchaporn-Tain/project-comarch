#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std; 

// โครงสร้างสำหรับเก็บข้อมูล 1 คำสั่งที่อ่านได้จากไฟล์
struct MemoryAccess {
    int type;             // 0 = Read, 1 = Write
    unsigned int address; // ที่อยู่หน่วยความจำ (Address)
    bool valid;           // ตัวเช็คว่าบรรทัดนี้อ่านข้อมูลสำเร็จหรือไม่
};

class TraceParser {
private:
    ifstream file;
    string filename;
    int lineCount;
    int errorCount;

public:
    // Constructor: พยายามเปิดไฟล์ทันทีที่สร้าง Object
    TraceParser(string fname) {
        filename = fname;
        lineCount = 0;
        errorCount = 0;
        
        file.open(filename);
        if (!file.is_open()) {
            cout << "[Error] Cannot open file: " << filename << endl;
        } else {
            cout << "[System] Successfully opened: " << filename << endl;
        }
    }

    // Destructor: ปิดไฟล์อัตโนมัติเมื่อทำงานเสร็จ
    ~TraceParser() {
        if (file.is_open()) {
            file.close();
            cout << "[System] File closed. Total lines read: " << lineCount 
                 << ", Errors skipped: " << errorCount << endl;
        }
    }

    // ฟังก์ชันหลัก: ดึงข้อมูลออกมาทีละ 1 บรรทัด (Streaming) ป้องกัน RAM เต็ม
    MemoryAccess getNextAccess() {
        MemoryAccess mem;
        mem.valid = false; // ตั้งค่าเริ่มต้นเป็น false ไว้ก่อน
        
        string line;
        
        // วนลูปอ่านไฟล์ไปเรื่อยๆ จนกว่าจะได้ข้อมูลที่ถูกต้อง 1 บรรทัด หรือจนกว่าไฟล์จะหมด
        while (getline(file, line)) {
            lineCount++;
            
            // 1. ข้ามบรรทัดที่ว่างเปล่า
            if (line.empty()) continue;
            
            // 2. ข้ามบรรทัดที่เป็น Comment (ไฟล์เทรซมักจะใช้ # หรือ // อธิบายสเปคไฟล์)
            if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) continue;

            // 3. จัดการข้อมูล (Data Extraction & Error Handling)
            stringstream ss(line);
            int type;
            unsigned int address;
            
            // ลองใช้ stringstream ดึงข้อมูลเป็นตัวเลขฐานสิบ (type) และฐานสิบหก (hex address)
            if (ss >> type >> hex >> address) {
                mem.type = type;
                mem.address = address;
                mem.valid = true;
                return mem; // ส่งค่ากลับทันทีที่ได้ข้อมูลครบ
            } else {
                // ถ้าดึงข้อมูลไม่ได้ (เช่น บรรทัดนั้นข้อมูลพัง พิมพ์ตัวอักษรผิดมา) โปรแกรมจะไม่ค้าง แต่จะข้ามไป
                errorCount++;
                cout << "[Warning] Data format error at line " << lineCount 
                     << " -> \"" << line << "\". Skipping..." << endl;
            }
        }
        
        // ถ้าหลุดลูป while ออกมาได้ แสดงว่าไฟล์หมดแล้ว (EOF) จะคืนค่า mem.valid = false กลับไป
        return mem;
    }

    // ฟังก์ชันเช็คว่าไฟล์ยังเปิดอยู่และพร้อมอ่านไหม
    bool isReady() {
        return file.is_open() && !file.eof();
    }
};

// =========================================================
// ฟังก์ชันเสริม: สร้างไฟล์ Trace จำลอง เอาไว้ให้เพื่อนในทีมทดสอบระบบ
// =========================================================
void createDummyTraceFile(string filename) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << "# Dummy Memory Trace for Testing\n";
        outFile << "# Format: <Type: 0=Read, 1=Write> <Address: Hex>\n\n";
        
        outFile << "0 0000\n";     // Read Address 0x0000
        outFile << "1 0004\n";     // Write Address 0x0004
        outFile << "BAD_DATA_123\n"; // แกล้งจำลองข้อมูลที่พัง เพื่อเทสต์ระบบ Error Handling
        outFile << "0 0008\n";
        outFile << "0 0400\n";
        outFile << "# End of trace\n";
        
        outFile.close();
        cout << "[System] Created dummy trace file: " << filename << endl;
    }
}

int main() {
    string testFilename = "test_trace.txt";
    
    // 1. สร้างไฟล์จำลองเพื่อเตรียมทดสอบ
    createDummyTraceFile(testFilename);
    cout << "-----------------------------------\n";

    // 2. เริ่มทำงานโมดูล TraceParser
    TraceParser parser(testFilename);
    
    if (!parser.isReady()) {
        return 1; // จบโปรแกรมทันทีถ้าหาไฟล์ไม่เจอ
    }

    cout << "\n--- Start Parsing ---\n";
    
    // 3. ลูปหลักที่ System Integrator จะนำไปประกอบร่างกับ CacheSimulator
    while (true) {
        MemoryAccess mem = parser.getNextAccess();
        
        // ถ้า valid เป็น false แปลว่าอ่านจบไฟล์แล้ว ให้เบรคออกจากลูป
        if (mem.valid == false) {
            break; 
        }

        // --- จุดเชื่อมต่อระบบ ---
        // ตรงนี้คือที่ที่จะเอา mem.address โยนเข้าไปในลอจิกของเพื่อนคนที่ทำ Core Architect
        // เช่น: myCache.accessMemory(mem.address);
        
        cout << "Parsed -> Type: " << mem.type 
             << " | Address: 0x" << hex << mem.address << dec << endl;
    }
    cout << "--- End Parsing ---\n\n";

    return 0;
}