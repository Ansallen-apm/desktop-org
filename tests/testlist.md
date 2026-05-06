# 桌面整理工具 (desktop-org) 測試計畫清單

本清單列出專案中適合進行單元測試 (Unit Test) 的核心模組與待測項目。
這些模組屬於**綠燈區**（低系統依賴、純邏輯計算），非常適合自動化測試防呆。

## 🟢 模組 1：ZoneManager (區塊管理與幾何計算)
主要負責區塊的管理、繪製快取與滑鼠碰撞偵測 (HitTest)。

- [x] **`HitTest_Inside`**: 測試給定落在區塊內的 `POINT`，是否能正確回傳該區塊的 index。
- [x] **`HitTest_Outside`**: 測試給定在所有區塊外的 `POINT`，是否回傳 `-1`。
- [ ] **`HitTestEdge`**: 測試邊緣判定。在距離邊界 < 8 像素時，是否回傳正確的 Bitmask (例如：Top-Left 應該包含 1(L) 和 2(T))。
- [ ] **`HitTestEdge_Inner`**: 測試在區塊內部，但距離邊緣大於 8 像素時，EdgeMask 應為 0。
- [ ] **`AddZone`**: 測試新增區塊後，Count 是否正確增加，且顏色、名稱、副檔名預設值是否正確寫入。

## 🟢 模組 2：ConfigManager (設定檔存取)
負責 `config.ini` 的序列化與反序列化。為了避免污染真實的 AppData，測試時應使用 Mock 路徑或暫存檔。

- [ ] **`SaveAndLoadZones`**: 寫入多個包含特殊檔名、多重副檔名、極端顏色值的 `ZoneConfig` 陣列，儲存後再次讀取，驗證所有屬性是否 100% 吻合。
- [ ] **`EmptyConfig`**: 測試在沒有 `config.ini` 的環境下執行 `LoadZones`，是否能安全地回傳空的 `std::vector` 而不拋出例外。

## 🟢 模組 3：AutoSorter (字串與規則處理)
由於 `RunSort` 高度依賴 `IconManager`，單元測試主要針對規則解析與字串比對功能。

- [ ] **`GetExtension_Normal`**: 測試正常檔名如 `"document.pdf"` 是否能提煉出 `".pdf"`。
- [ ] **`GetExtension_CaseInsensitive`**: 測試大寫後綴如 `"image.JPG"` 是否能提煉出並正規化為小寫 `".jpg"`。
- [ ] **`GetExtension_NoExt`**: 測試無副檔名的檔案 (如 `"folder"`)，是否能安全回傳空字串 `""`。
- [ ] **`MapExtensionToZone`**: 測試規則註冊機制，包含空白清理與重複註冊覆蓋行為。

## ⚠️ 黃/紅燈區 (整合測試 / 不適合單元測試)
以下項目高度依賴 Windows 底層狀態，建議以人工 QA 或自動化端到端腳本進行驗證：
1. **`IconManager`**: IPC 跨 Process 讀取 ListView，依賴 Explorer.exe。
2. **`DesktopHooker`**: 注入 WorkerW，依賴作業系統層級的多螢幕環境與 Z-Order。
3. **`main.cpp` (WinMain / WindowProc)**: UI 滑鼠拖曳互動、System Tray 常駐行為。
