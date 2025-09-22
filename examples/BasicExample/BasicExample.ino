#include "FSNextionLib.h"

FSNextionLib nextion(Serial2);

void handleTouch(byte pageId, byte componentId, byte eventType) {
    Serial.printf("Touch Event: Page=%d, Component=%d, Event=%s\n", 
                 pageId, componentId, eventType == 0 ? "Release" : "Press");
    
    // Örnek: Sayfa 0, Bileşen 1'e tıklanınca
    if (pageId == 0 && componentId == 1 && eventType == 0) {
        nextion.setText("t0", "Merhaba Dünya!");
        nextion.setNumber("n0", 1234);
        nextion.setPage(1);
    }
}

void handleSystemEvent(byte eventType) {
    Serial.printf("System Event: 0x%02X\n", eventType);
    if (eventType == 0x86) {
        Serial.println("Nextion uyku moduna geçti");
    } else if (eventType == 0x87) {
        Serial.println("Nextion uyandı");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Nextion Kütüphane Testi Başlıyor...");
    
    // Nextion'ı başlat
    nextion.begin(115200);
    nextion.setDebug(true);
    nextion.setTimeout(1000);
    
    // Callback'leri kaydet
    nextion.onTouch(handleTouch);
    nextion.onSystemEvent(handleSystemEvent);
    
    // Bağlantı kontrolü
    if (nextion.isConnected()) {
        Serial.println("Nextion bağlantısı başarılı!");
        
        // Başlangıç ayarları
        nextion.setPage(0);
        nextion.setText("t0", "Hazır!");
        nextion.setBrightness(100);
    } else {
        Serial.println("Nextion bağlantısı başarısız!");
    }
}

void loop() {
    // Sürekli olarak event'leri dinle
    nextion.listen();
    
    // Örnek: Her 10 saniyede bir veri güncelle
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 10000) {
        lastUpdate = millis();
        
        // Sıcaklık değeri güncelle (örnek)
        int temperature = random(20, 30);
        nextion.setNumber("tempVal", temperature);
        
        // Progress bar güncelle
        static byte progress = 0;
        progress = (progress + 10) % 100;
        nextion.setProgress("progressBar", progress);
        
        Serial.println("Veri güncellendi: Sıcaklık=" + String(temperature));
    }
    
    // Hata kontrolü
    if (nextion.getLastError() != FSNextionLib::ErrorCode::SUCCESS) {
        Serial.println("Hata kodu: " + String((int)nextion.getLastError()));
        nextion.clearError();
    }
    
    delay(10); // Küçük bir gecikme
}