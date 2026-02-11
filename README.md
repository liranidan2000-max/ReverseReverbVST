# Reverse Reverb VST3 + Standalone Plugin

פלאגין VST3 ואפליקציה עצמאית (Standalone) שמאפשר לטעון דגימת אודיו, להוסיף לה ריוורב, לבצע ריוורס ולנגן את התוצאה.

## תכונות

- **פורמטים**: VST3 Plugin + Standalone App
- **טעינת קבצים**: תמיכה ב-WAV, MP3, AIFF, FLAC
- **מגבלת זמן**: עד 8 שניות לדגימה
- **Drag & Drop**: גרירת קבצים ישירות לממשק
- **MIDI Trigger**: הפעלת הסאמפל בעזרת MIDI notes
- **פרמטרים**:
  - Room Size - גודל החדר של הריוורב
  - Reverb Mix - כמות הריוורב (dry/wet של הריוורב)
  - Output Level - עוצמת הפלט הכוללת

## איך זה עובד

1. **טעינה**: גרור קובץ אודיו לאזור המיועד או לחץ "Browse File"
2. **עיבוד**: הפלאגין אוטומטית מוסיף ריוורב ומבצע ריוורס
3. **התאמת פרמטרים**: סובב את הכפתורים כדי לשנות את הסאונד
4. **עיבוד מחדש**: לחץ "Process Reverb" אחרי שינוי פרמטרים
5. **נגינה**: 
   - לחץ "Play Sample" בממשק
   - או שלח MIDI note מה-DAW שלך

## הוראות Build

### דרישות
- JUCE Framework (גרסה 7.0 ומעלה)
- Xcode (macOS)
- Projucer

### שלבים

1. **פתח את Projucer**
   - הפעל את Projucer
   - פתח את הקובץ `ReverseReverb.jucer`

2. **הגדר נתיב JUCE**
   - ב-Projucer, לך ל-Settings
   - הגדר את "Global Paths" → "Path to JUCE"
   - הצבע לתיקייה שבה התקנת את JUCE

3. **שמור ופתח ב-Xcode**
   - ב-Projucer, לחץ על "Save Project and Open in IDE"
   - זה יפתח אוטומטית את Xcode

4. **Build**
   - ב-Xcode יהיו לך 2 Schemes:
     - **ReverseReverb - VST3** (לפלאגין)
     - **ReverseReverb - Standalone Plugin** (לאפליקציה עצמאית)
   
   **לבניית VST3:**
   - בחר Scheme: "ReverseReverb - VST3"
   - Build Configuration: "Release"
   - לחץ Cmd+B
   
   **לבניית Standalone:**
   - בחר Scheme: "ReverseReverb - Standalone Plugin"
   - Build Configuration: "Release"
   - לחץ Cmd+B

5. **התקנה**
   
   **VST3 Plugin:**
   - הקובץ ישמר ב: `Builds/MacOSX/build/Release/ReverseReverb.vst3`
   - העתק ל: `~/Library/Audio/Plug-Ins/VST3/`
   
   **Standalone App:**
   - הקובץ ישמר ב: `Builds/MacOSX/build/Release/ReverseReverb.app`
   - העתק ל: `~/Applications/` או הפעל ישירות מהמיקום

## שימוש

### שימוש ב-Standalone App

1. **הפעל את האפליקציה** ReverseReverb.app
2. **טען דגימה** - Drag & Drop או לחץ "Browse File"
3. **התאם פרמטרים** - Room Size, Reverb Mix, Output Level
4. **עבד מחדש** - לחץ "Process Reverb" אחרי שינוי
5. **נגן** - לחץ "Play Sample" או חבר MIDI keyboard

**הגדרות אודיו ב-Standalone:**
- Options → Audio/MIDI Settings
- בחר Audio Device (Built-in, Audio Interface, וכו')
- בחר MIDI Input אם יש לך keyboard

### שימוש ב-DAW (VST3)

1. פתח את ה-DAW שלך (Ableton, Logic, FL Studio, וכו')
2. טען את הפלאגין על track חדש
3. טען דגימה (drag & drop או browse)
4. התאם פרמטרים
5. נגן MIDI notes לטריגור הסאמפל

## טיפים

- **סאונדים אטמוספריים**: השתמש ב-Room Size גבוה + Reverb Mix גבוה
- **גלישות הפוכות**: טען צליל קצר וחד (snare, clap)
- **טקסטורות**: נסה דגימות של קולות/שירה
- **אוטומציה**: אתה יכול לאוטומט את הפרמטרים ב-DAW

## פתרון בעיות

### הפלאגין לא מופיע ב-DAW
- ודא שהקובץ .vst3 נמצא בתיקיית VST3 הנכונה
- סרוק מחדש plugins ב-DAW
- בדוק שה-DAW תומך ב-VST3

### אין סאונד
- ודא שטענת דגימה
- בדוק שה-Output Level לא ב-0
- נסה ללחוץ "Play Sample" או לשלוח MIDI note

### קובץ לא נטען
- ודא שהקובץ קצר מ-8 שניות
- נסה פורמט אחר (WAV הכי יציב)

## מבנה הקוד

```
Source/
├── PluginProcessor.h      - הגדרות מעבד האודיו
├── PluginProcessor.cpp    - לוגיקת עיבוד + ריוורב + ריוורס
├── PluginEditor.h         - הגדרות ממשק משתמש
└── PluginEditor.cpp       - ממשק גרפי + drag & drop
```

## התאמות אישיות אפשריות

אם אתה רוצה לשנות משהו:

1. **שינוי מקסימום זמן דגימה**:
   - `PluginProcessor.cpp` → שורה ~165
   - שנה `if (duration > 8.0)` למשך אחר

2. **שינוי צבעי UI**:
   - `PluginEditor.cpp` → פונקציית `paint()`

3. **הוספת פרמטרים**:
   - הוסף sliders ב-`PluginEditor.cpp`
   - הוסף משתנים ב-`PluginProcessor.h`

## רישיון

פרויקט זה נבנה עם JUCE Framework.
JUCE זמין תחת רישיון GPL או רישיון מסחרי.

---

**נבנה ב-2026 | VST3 Plugin for Reverse Reverb Effect**
