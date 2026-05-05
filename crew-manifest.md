### A. 專案簡介 (Project Brief)
`desktop-org` 是一款為辦公室白領設計的桌面圖示分類與收納軟體（類似 Stardock Fences）。專案採用 C++ 開發，以確保極低的系統資源佔用與原生體驗。核心目標是實作「自訂分類區塊」與「圖示位置固定」，並將自動歸類、快捷鍵隱藏與視覺自訂等進階功能安排於開發後期。

### B. 出團專家名錄 (The Roster)
- **@cpp-pro (C++ 專家)**：負責核心邏輯、Windows 底層 API 呼叫（Desktop Hooking、圖示操作）與效能最佳化。
- **@senior-architect (系統架構師)**：負責 C++ 專案結構設計、編譯環境（CMake / MSVC）搭建，確保架構穩定擴展。
- **@ui-ux-pro-max (UI/UX 專家)**：負責桌面區塊（Fences）的透明視窗設計、視覺樣式與使用者互動邏輯。

### C. 作戰執行步驟 (Execution Plan)
- **Step 1:** 專案基礎建設與技術選型（CMake + Qt/Win32） - 由 `@senior-architect` 負責
- **Step 2:** 系統架構設計與 Windows 桌面 Hooking 研究 - 由 `@cpp-pro` 負責
- **Step 3:** 建立基礎透明覆蓋視窗 (Overlay Window) - 由 `@ui-ux-pro-max` 負責
- **Step 4:** 核心功能一：實作桌面區塊 (Zone/Fence) 的繪製與拖曳機制 - 由 `@cpp-pro` 負責
- **Step 5:** 核心功能二：實作桌面圖示的攔截與固定定位 (Pin to Position) - 由 `@cpp-pro` 負責
*(--- Checkpoint 1 ---)*
- **Step 6:** 進階功能一：副檔名自動分類機制 - 由 `@cpp-pro` 負責
- **Step 7:** 進階功能二：雙擊桌面/快捷鍵顯示與隱藏功能 - 由 `@cpp-pro` 負責
- **Step 8:** 進階功能三：區塊視覺自訂面板（顏色、透明度、標題） - 由 `@ui-ux-pro-max` 負責
- **Step 9:** 最終測試、效能分析與編譯打包 - 由 `@senior-architect` 負責

### D. 模型建議 (Model Strategy)
- **Step 1, 3, 8, 9**: ⚡ **Flash** (適合：環境搭建、單純 UI 邏輯與測試報告)
- **Step 2, 4, 5, 6, 7**: 🧠 **Pro / Thinking** (適合：複雜 Windows API Hooking、底層記憶體操作與演算法)

### E. 安全機制 (Safety Rules)
- **Max Retries**: 同一步驟修正超過 **3 次**仍失敗 → 自動標記為 `⚠️ BLOCKED`，跳過該步驟，將未解問題寫入 `handoff.md` 並通知老闆。
- **Checkpoint 頻率**: 每完成 **5 個步驟**就強制暫停，自動執行一次 `/handoff` 儲存斷點，並匯報進度詢問是否要繼續。
- **模型切換安全閥**: 當即將從 Flash 切換到 Pro（或反過來）時，**必須先自動執行 `/handoff`** 保存當前進度，防止上下文截斷導致記憶遺失。

### F. 成本與複雜度預估 (Estimated Cost)
由於涉及 Windows Desktop 底層 API，技術門檻較高。
| 階段 | 複雜度 | 模型建議 |
|------|--------|----------|
| 規劃期 (Step 1-2) | 🟡 Med | Flash / Pro |
| 核心開發 (Step 3-5) | 🔴 High | Pro |
| 進階與優化 (Step 6-9) | 🟡 Med | Flash / Pro |

### G. Token 預算 (Token Budget)
- **預估 API 呼叫量**：`總步驟數(9) × 每步平均 3 次 LLM 呼叫 = 27 次`
- **Hard Cap (硬性上限)**：**100 次**
- **超限動作**：當 `/execute-crew` 的 `api_call_count` 超過 Hard Cap 時：
  1. 立即暫停所有實作行為。
  2. 自動執行 `/handoff` 保存當前進度。
  3. 向使用者報告：「⚠️ **預算上限到達**：已消耗 {api_call_count} 次 API 呼叫，達到 Hard Cap (100)。進度已自動保存。老闆，是否要追加預算繼續？」
