#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

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
    
    // อาเรย์ 2 มิติ เก็บข้อมูล Cache
    vector<vector<CacheBlock>> cache;

public:
    // Constructor
    CacheSimulator(int cSize, int bSize, int w, int p) {
        cacheSize = cSize;
        blockSize = bSize;
        ways = w;
        policy = p; 
        
        globalTime = 0;
        hit_count = 0;
        miss_count = 0;
        
        numSets = cacheSize / (blockSize * ways);
        
        offsetBits = (int)log2(blockSize);
        if (numSets > 1) {
            indexBits = (int)log2(numSets);
        } else {
            indexBits = 0; 
        }
        
        // จองพื้นที่ให้ Vector และรีเซ็ตค่าเริ่มต้น
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
        globalTime++; 
        
        unsigned int indexMask = (1 << indexBits) - 1;
        unsigned int index = (address >> offsetBits) & indexMask;
        unsigned int tag = address >> (offsetBits + indexBits);

        // 1. ตรวจสอบ HIT
        for (int i = 0; i < ways; i++) {
            if (cache[index][i].valid == true && cache[index][i].tag == tag) {
                hit_count++;
                if (policy == 0) { // LRU
                    cache[index][i].lastUsed = globalTime; 
                }
                return; 
            }
        }

        // 2. ถ้า MISS หาช่องว่าง
        miss_count++;
        int targetWay = -1; 

        for (int i = 0; i < ways; i++) {
            if (cache[index][i].valid == false) {
                targetWay = i;
                break;
            }
        }

        // 3. ถ้าแคชเต็ม (ต้อง Replace)
        if (targetWay == -1) {
            targetWay = 0; 
            long long oldestTime;
            
            if (policy == 0) oldestTime = cache[index][0].lastUsed;
            else oldestTime = cache[index][0].loadTime;

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

        // 4. อัปเดตข้อมูลใหม่ลงในช่องที่เลือก
        cache[index][targetWay].valid = true;
        cache[index][targetWay].tag = tag;
        cache[index][targetWay].loadTime = globalTime; 
        cache[index][targetWay].lastUsed = globalTime; 
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
        cout << "-------------------\n";
    }
};

#endif // CACHE_SIMULATOR_H