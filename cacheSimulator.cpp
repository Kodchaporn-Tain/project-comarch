#include <iostream>
#include <vector>
#include <cmath>

using namespace std; // ปี 2 ชอบใช้สิ่งนี้จะได้ไม่ต้องพิมพ์ std:: บ่อยๆ

// โครงสร้างของ Block ข้อมูลใน Cache
struct CacheBlock {
    bool valid;
    unsigned int tag;
    long long lastUsed; // เวลาที่โดนเรียกใช้ล่าสุด (สำหรับ LRU)
    long long loadTime; // เวลาที่ถูกโหลดเข้ามาในแคช (สำหรับ FIFO)
};

class CacheSimulator {
private:
    int cacheSize;
    int blockSize;
    int ways;
    int numSets;
    
    int offsetBits;
    int indexBits;
    
    long long globalTime; // ตัวนับเวลาจำลอง (Clock)
    
    int hit_count;
    int miss_count;
    
    int policy; // 0 = LRU, 1 = FIFO
    
    // ใช้ vector 2 มิติแทนการสร้าง struct ซ้อนกันให้วุ่นวาย
    // แถว (Row) คือ Index ของ Set, คอลัมน์ (Col) คือ Ways ใน Set นั้น
    vector<vector<CacheBlock>> cache;

public:
    // Constructor (เขียนกำหนดค่าแบบตรงๆ เข้าใจง่าย)
    CacheSimulator(int cSize, int bSize, int w, int p) {
        cacheSize = cSize;
        blockSize = bSize;
        ways = w;
        policy = p; 
        
        globalTime = 0;
        hit_count = 0;
        miss_count = 0;
        
        // คำนวณจำนวน Set
        numSets = cacheSize / (blockSize * ways);
        
        // คำนวณบิตโดยใช้ log2 แล้วแปลงเป็น int
        offsetBits = (int)log2(blockSize);
        if (numSets > 1) {
            indexBits = (int)log2(numSets);
        } else {
            indexBits = 0; // กรณี Fully Associative จะไม่มี Index
        }
        
        // จองพื้นที่ให้ Vector และกำหนดให้ valid = false ทุกช่องตอนเริ่ม
        cache.resize(numSets);
        for (int i = 0; i < numSets; i++) {
            cache[i].resize(ways);
            for (int j = 0; j < ways; j++) {
                cache[i][j].valid = false;
                cache[i][j].tag = 0;
                cache[i][j].lastUsed = 0;
                cache[i][j].loadTime = 0;
            }
        }
    }

    // ฟังก์ชันจำลองการเรียกใช้งานหน่วยความจำ
    void accessMemory(unsigned int address) {
        globalTime++; // เวลาเดินไป 1 step
        
        // 1. แยกส่วน Address (คำนวณ Index กับ Tag)
        unsigned int indexMask = (1 << indexBits) - 1;
        unsigned int index = (address >> offsetBits) & indexMask;
        unsigned int tag = address >> (offsetBits + indexBits);

        // 2. เช็คว่า HIT ไหม (วนลูปหาใน Set นั้นๆ)
        for (int i = 0; i < ways; i++) {
            if (cache[index][i].valid == true && cache[index][i].tag == tag) {
                hit_count++;
                
                // ถ้าเป็น LRU (0) ต้องอัปเดตเวลาว่าเพิ่งโดนเรียกใช้
                if (policy == 0) {
                    cache[index][i].lastUsed = globalTime; 
                }
                // ถ้าเป็น FIFO (1) ไม่ต้องทำอะไร เพราะเราสนแค่ตอนโหลดเข้ามา
                
                return; // เจอแล้วก็จบการทำงานเลย
            }
        }

        // 3. ถ้าลูปข้างบนไม่ return แสดงว่า MISS แน่นอน
        miss_count++;
        
        int targetWay = -1; // ตัวแปรเก็บตำแหน่งช่องที่เราจะเอาข้อมูลไปใส่

        // 3.1 ลองหาช่องว่างดูก่อน (ช่องที่ valid == false)
        for (int i = 0; i < ways; i++) {
            if (cache[index][i].valid == false) {
                targetWay = i;
                break;
            }
        }

        // 3.2 ถ้าแคชเต็ม (targetWay ยังเป็น -1) ต้องเตะของเก่าออก
        if (targetWay == -1) {
            targetWay = 0; // สมมติให้ช่องแรกเก่าสุดไปก่อน
            long long oldestTime;
            
            if (policy == 0) {
                oldestTime = cache[index][0].lastUsed;
            } else {
                oldestTime = cache[index][0].loadTime;
            }

            // วนหาช่องที่เวลาเก่าที่สุด
            for (int i = 1; i < ways; i++) {
                if (policy == 0) { // LRU
                    if (cache[index][i].lastUsed < oldestTime) {
                        oldestTime = cache[index][i].lastUsed;
                        targetWay = i;
                    }
                } 
                else if (policy == 1) { // FIFO
                    if (cache[index][i].loadTime < oldestTime) {
                        oldestTime = cache[index][i].loadTime;
                        targetWay = i;
                    }
                }
            }
        }

        // 4. เอาข้อมูลใหม่ใส่ลงไปในช่อง targetWay
        cache[index][targetWay].valid = true;
        cache[index][targetWay].tag = tag;
        cache[index][targetWay].loadTime = globalTime; // อัปเดตเวลาตอนโหลด
        cache[index][targetWay].lastUsed = globalTime; // อัปเดตเวลาใช้งานล่าสุด
    }

    // ฟังก์ชันปริ้นท์สรุปผล
    void printStats() {
        int total = hit_count + miss_count;
        double hit_rate = 0.0;
        
        if (total > 0) {
            hit_rate = ((double)hit_count / total) * 100;
        }
        
        if (policy == 0) cout << "--- Policy: LRU ---\n";
        else cout << "--- Policy: FIFO ---\n";
        
        cout << "Total Access : " << total << endl;
        cout << "Hits         : " << hit_count << endl;
        cout << "Misses       : " << miss_count << endl;
        cout << "Hit Rate     : " << hit_rate << " %" << endl;
        cout << "-------------------\n\n";
    }
};

int main() {
    // สร้างชุดคำสั่งทดสอบเบื้องต้น
    vector<unsigned int> traces = { 
        0x0000, 0x0010, 0x0020, 0x0030, // ใส่ข้อมูลเข้าไปให้เต็มแคช
        0x0000,                         // เรียกซ้ำ
        0x0040                          // ใส่ข้อมูลใหม่ ดันของเก่าออก
    };

    cout << "Testing 2-Way Set Associative (Cache Size: 64B, Block Size: 16B)\n\n";

    // จำลองแบบ LRU (policy = 0)
    CacheSimulator lruCache(64, 16, 2, 0);
    for (int i = 0; i < traces.size(); i++) {
        lruCache.accessMemory(traces[i]);
    }
    lruCache.printStats();

    // จำลองแบบ FIFO (policy = 1)
    CacheSimulator fifoCache(64, 16, 2, 1);
    for (int i = 0; i < traces.size(); i++) {
        fifoCache.accessMemory(traces[i]);
    }
    fifoCache.printStats();

    return 0;
}