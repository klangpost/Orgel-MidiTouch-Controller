#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

#include <lvgl.h>
//#include "lv_conf.h"

#include "USB.h"
#include "USBMIDI.h"
USBMIDI MIDI;

// ========================
// == Konfiguration LVGL ==
// ==============================================================================================================
class LGFX : public lgfx::LGFX_Device
{
public:

lgfx::Bus_RGB _bus_instance;
lgfx::Panel_RGB _panel_instance;
lgfx::Light_PWM _light_instance;
lgfx::Touch_GT911 _touch_instance;

LGFX(void)
{
{
auto cfg = _panel_instance.config();

cfg.memory_width = 800;
cfg.memory_height = 480;
cfg.panel_width = 800;
cfg.panel_height = 480;

cfg.offset_x = 0;
cfg.offset_y = 0;

_panel_instance.config(cfg);
}

{
auto cfg = _panel_instance.config_detail();

cfg.use_psram = 1;

_panel_instance.config_detail(cfg);
}

{
auto cfg = _bus_instance.config();
cfg.panel = &_panel_instance;
cfg.pin_d0 = GPIO_NUM_14; // D0
cfg.pin_d1 = GPIO_NUM_38; // D1
cfg.pin_d2 = GPIO_NUM_18; // D2
cfg.pin_d3 = GPIO_NUM_17; // D3
cfg.pin_d4 = GPIO_NUM_10; // D4
cfg.pin_d5 = GPIO_NUM_39; // D5
cfg.pin_d6 = GPIO_NUM_0; // D6
cfg.pin_d7 = GPIO_NUM_45; // D7
cfg.pin_d8 = GPIO_NUM_48; // D8
cfg.pin_d9 = GPIO_NUM_47; // D9
cfg.pin_d10 = GPIO_NUM_21; // D10
cfg.pin_d11 = GPIO_NUM_1; // D11
cfg.pin_d12 = GPIO_NUM_2; // D12
cfg.pin_d13 = GPIO_NUM_42; // D13
cfg.pin_d14 = GPIO_NUM_41; // D14
cfg.pin_d15 = GPIO_NUM_40; // D15

cfg.pin_henable = GPIO_NUM_5; // DE
cfg.pin_vsync = GPIO_NUM_3; // VSYNC
cfg.pin_hsync = GPIO_NUM_46; // HSYNC
cfg.pin_pclk = GPIO_NUM_7; // PCLK
cfg.freq_write = 16000000; // 16 MHz

cfg.hsync_polarity = 0;
cfg.hsync_front_porch = 8;
cfg.hsync_pulse_width = 4;
cfg.hsync_back_porch = 8;
cfg.vsync_polarity = 0;
cfg.vsync_front_porch = 8;
cfg.vsync_pulse_width = 4;
cfg.vsync_back_porch = 8;
cfg.pclk_idle_high = 0;
_bus_instance.config(cfg);
}
_panel_instance.setBus(&_bus_instance);

{
auto cfg = _light_instance.config();
cfg.pin_bl = GPIO_NUM_2; // Anpassen, falls notwendig
_light_instance.config(cfg);
}
_panel_instance.light(&_light_instance);

 {
 auto cfg = _touch_instance.config();
  cfg.x_min      = 0;
  cfg.y_min      = 0;
  cfg.bus_shared = false;
  cfg.offset_rotation = 0;
  // I2C connection
  cfg.i2c_port   = I2C_NUM_0;
  cfg.pin_sda    = GPIO_NUM_8;
  cfg.pin_scl    = GPIO_NUM_9;
  cfg.pin_int    = GPIO_NUM_NC;
  cfg.pin_rst    = GPIO_NUM_38;
  cfg.x_max      = 800;
  cfg.y_max      = 480;
  cfg.freq       = 100000;
  _touch_instance.config(cfg);
  _panel_instance.setTouch(&_touch_instance);
        }

setPanel(&_panel_instance);
}
};

LGFX lcd;

static const uint16_t screenWidth = 800;
static const uint16_t screenHeight = 480;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/*** Funktionen deklarieren ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void lv_button_demo(void);
void lv_button_demo_tabview(void);
void sendNoteON(byte note);
void sendNoteOFF(byte note);
static void button_event_handler(lv_event_t * e);
static void setzer_button_event_handler(lv_event_t * e);

static int32_t x, y, number;
static int32_t testcounter = 0;

// Globale Variablen (TabVIEW)
lv_obj_t *tabview;
lv_obj_t *tab_register;
lv_obj_t *tab_setzer;
lv_obj_t *tab_setup;

// **Kombinierte Arrays für Button-Texte !!!TEMP!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Arrays für Orgelnamen und zugehörige Button-Texte (für Setup Tab)
char *organ_names[] = {
    "Organ 1", // Beispielname für Orgel 1
    "Organ 2", // Beispielname für Orgel 2
    "Organ 3"  // Beispielname für Orgel 3
};

// Orgel 1
char *buttonText[] = {
    "Prinzipal 8", "Hohlfloete 8", "Oktave 4", "Blockfloete 4", "Oktave 2", "Flageolett 2", "Cornet 4f",
    "Mixtur 4-6f", "Trompete 8", " ", " ", " ", " ", "Koppel 1/2",

    "Gedackt 8", "Gambe 8", "Prinzipal 4", "Rohrfloete 4", "Oktave 2", "Nasat 1 1/3",
    "Sesquialtera 2f", "Scharff 3-4f", "Cromoron 8", "Tremulant", " ",  " ", " ", " ",

    "Subbass 16.", "Pinzipal 8", "Gemhorn 8", "Oktave 4", "Posaune 16", "Trompete 8",
    " ", " ", " ", " ", " ", " ", "Koppel P/1", "Koppel P/II"
};

// Orgel 2
char *buttonText1[] = {
   "Principal 8", "Quintadena 8", "Rohrfloete 8", "Octava 4", "Spitzfloethe 4", "Quinta 3", "Octava 2",
    "Mixtur IV", "Cornet III", " ", " ", " ", " ", "Koppel 1/2",

    "Gedackt 8", "Rohrfloete 4", "Nassat 3", "Octava 2", "Tertia 1 3/5", "Quinta 1 1/2",
    "Sufflet 1", "Cimbel II", " ", "Tremulant", " ",  " ", " ", " ",

    "Subbass 16.", "Octavbass 8", "Posaune 16", " ", " ", " ",
    " ", " ", " ", " ", " ", " ", "Koppel P/1", "Koppel P/II"
};

// Orgel 3
char *buttonText2[] = {
    " ", " ", " ", " ", " ", " ", " ",
    " ", " ", " ", " ", " ", " ", " ",

    " ", " ", " ", " ", " ", " ",
    " ", " ", " ", "", " ",  " ", " ", " ",

    " ", " ", " ", " ", " ", " ",
    " ", " ", " ", " ", " ", " ", " ", " "
};
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

const byte midi_codes[] = {
    48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61,

    62, 63, 64, 65, 66, 67, 68,
    69, 70, 80, 81, 82, 83, 84,

    71, 72, 73, 74, 75, 76, 77,
    78, 79, 85, 86, 87, 88, 89
};

const byte setzer_midi_notes[] = {
  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111
};

char *setzer_button_texts[] = { // Texte für Setzer Buttons
  "SET", "1", "2", "3", "4", "5", "6",
  "7", "8", "9", "CC"
};

// **Global Variable, um den aktuell ausgewählten Orgelnamen zu speichern**
char *current_organ_name = organ_names[0]; // Startet standardmäßig mit der ersten Orgel
char **current_buttonText = buttonText; // Startet standardmäßig mit den Button-Texten der ersten Orgel

lv_obj_t *RegisterButtons[7 * 6]; // Array für ALLE Register Buttons (7 Zeilen * 6 Spalten = 42 Buttons)

// Forward Declarations
//void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
//void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
//void lv_button_demo(void);

void setup()
{
  Serial.begin(115200);
  lcd.init();
  lv_init();

  lcd.setRotation(0);
  lcd.fillScreen(TFT_BLACK);

  /* LVGL : Einrichten des Buffer für das Display */
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*** LVGL : Setup & Initialize display device driver ***/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*** LVGL : Setup & Initialize input device driver ***/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Midi beginnen
  MIDI.begin();
  USB.begin();
 
  // Oberfläche erstellen
  lv_button_demo_tabview();
}


void loop()
{
  lv_timer_handler(); /* GUI ihre Arbeit machen lassen */
  delay(5);
  lv_tick_inc(10);

  //prüfen auf Midi Event
  midiEventPacket_t midi_packet_in = {0, 0, 0, 0};
  if (MIDI.readPacket(&midi_packet_in)) 
  {
    // Midi Event liegt vor, bearbeiten!
    midi_in_handle(midi_packet_in);
  }
}


// ==========================================================================0

/*** Display callback to flush the buffer to screen ***/
  void display_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p)
  {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
  }

  /*** Touchpad callback to read the touchpad ***/
  void touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
  {
    uint16_t touchX, touchY;
    bool touched = lcd.getTouch(&touchX, &touchY);
    if (!touched)
    {
      data->state = LV_INDEV_STATE_REL;
    }
    else
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
    }
  }



// ====================================
// == Funktion zum erstellen der GUI ==
// =============================================================================================================================
void lv_button_demo_tabview(void) {
    lv_obj_t *label;

    // Layout Parameter
    int button_breite = 120;
    int button_hoehe = 50;
    int button_abstand_x = 5;
    int button_abstand_y = 4;
    int start_x = 10;
    int start_y_labels = 15;
    int start_y_buttons = 0;
    int label_abstand_y = 10;
    int bottom_space = 80;

    // Tabview erstellen
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
    tab_register = lv_tabview_add_tab(tabview, "Register");
    tab_setzer = lv_tabview_add_tab(tabview, "Setzer");
    tab_setup = lv_tabview_add_tab(tabview, "Setup");

    // Buttons erstellen im Tab "Register"
    for (int zeile = 0; zeile < 7; zeile++) {
        for (int spalte = 0; spalte < 6; spalte++) {
            lv_obj_t *btn;
            byte midi_note;
            int button_index = zeile + spalte * 7; // **Index für die kombinierten Arrays und das Button-Array berechnen**
            lv_obj_t *current_tab = tab_register; // Alle Register-Buttons sind im "Register" Tab

            // **JETZT NUR NOCH EIN Button Array: RegisterButtons!**
            btn = RegisterButtons[button_index] = lv_btn_create(current_tab);

            label = lv_label_create(btn);
            lv_label_set_text(label, buttonText[button_index]); // Text aus kombiniertem Array
            midi_note = midi_codes[button_index];               // MIDI-Code aus kombiniertem Array

            lv_obj_set_user_data(btn, (void*)(uintptr_t)midi_note);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_size(btn, button_breite, button_hoehe);
            lv_obj_set_pos(btn, start_x + spalte * (button_breite + button_abstand_x), start_y_buttons + zeile * (button_hoehe + button_abstand_y));
            lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_center(label);
        }
    }

    // Buttons erstellen im Tab "Setzer" - 11 Stück - Horizontale Anordnung
    int aktuelle_x_position = start_x;
    button_breite = 50;

    for (int i = 0; i < 11; i++) {
        lv_obj_t *btn;
        byte midi_note = setzer_midi_notes[i];
        Serial.println(midi_note);
        btn = lv_btn_create(tab_setzer);
        label = lv_label_create(btn);
        lv_label_set_text(label, setzer_button_texts[i]);

        lv_obj_set_user_data(btn, (void*)(uintptr_t)midi_note);
        lv_obj_set_size(btn, button_breite, button_hoehe);
        lv_obj_set_pos(btn, aktuelle_x_position, start_y_buttons);
        lv_obj_add_event_cb(btn, setzer_button_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_center(label);

        aktuelle_x_position += button_breite + button_abstand_x;
    }

  // **Buttons erstellen im Tab "Setup" - Orgel Auswahl Buttons - HORIZONTAL**
    lv_obj_t *setup_label = lv_label_create(tab_setup); // Label für Setup Tab
    lv_label_set_text(setup_label, "Orgel Auswahl:");
    lv_obj_align(setup_label, LV_ALIGN_TOP_LEFT, 10, 10);

    int aktuelle_x_position_setup = start_x; // Startposition für Setup Buttons
    int start_y_setup_buttons = 40; // Y-Position für Setup Buttons
    int setup_button_breite = 120;
    int setup_button_hoehe = 40;


    for (int i = 0; i < sizeof(organ_names) / sizeof(organ_names[0]); i++) { // Schleife über die Orgelnamen
        lv_obj_t *btn_setup_orgel;
        btn_setup_orgel = lv_btn_create(tab_setup); // Buttons im "Setup" Tab erstellen
        label = lv_label_create(btn_setup_orgel);
        lv_label_set_text(label, organ_names[i]); // Orgelnamen als Button-Beschriftung

        lv_obj_set_size(btn_setup_orgel, setup_button_breite, setup_button_hoehe);
        lv_obj_set_pos(btn_setup_orgel, aktuelle_x_position_setup, start_y_setup_buttons);
        // **WICHTIG:**  Hier den neuen Event Handler für die Setup Buttons hinzufügen!
        lv_obj_add_event_cb(btn_setup_orgel, setup_button_event_handler, LV_EVENT_CLICKED, (void*)organ_names[i]); // Orgelname als User Data übergeben!
        lv_obj_center(label);

        aktuelle_x_position_setup += setup_button_breite + button_abstand_x; // Horizontale Positionierung
    }
}

// ====================================
// == Funktion zum senden der NoteON ==
// =============================================================================================================================
void sendNoteON(byte note) {
  Serial.print("MIDI Note ON: ");
  Serial.println(note);
  MIDI.noteOn(note, 100);
}

// =====================================
// == Funktion zum senden der NoteOFF ==
// =============================================================================================================================
void sendNoteOFF(byte note) {
  Serial.print("MIDI Note OFF: ");
  Serial.println(note);
  MIDI.noteOff(note);
}


// =========================================================
// == Funktion zum berarbeiten der Events (Register VIEW) ==
// =============================================================================================================================
static void button_event_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e); // Button-Objekt bekommen

  if (event_code == LV_EVENT_VALUE_CHANGED) { // Geändert zu VALUE_CHANGED für Toggle Buttons
    lv_obj_t * label = lv_obj_get_child(obj, NULL); // Label des Buttons holen
    uintptr_t midi_note_ptr = (uintptr_t)lv_obj_get_user_data(obj); // Als uintptr_t holen (NEU)
    byte midi_note = (byte)midi_note_ptr; // Dann zu byte umwandeln (NEU)


    if (label) {
      Serial.print("Toggle Button '");
      Serial.print(lv_label_get_text(label)); // Button Beschriftung auslesen
      Serial.print("' wurde ");
       if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
          Serial.println("AUSGEWÄHLT (aktiviert)");
          // Sende Note ON, wenn Button ausgewählt
          sendNoteON(midi_note);
        } else {
          Serial.println("ABGEWÄHLT (deaktiviert)");
          // Sende Note OFF, wenn Button abgewählt
          sendNoteOFF(midi_note);
        }
    } else {
      Serial.println("Toggle Button (ohne Label) wurde getoggelt"); // Fallback
    }
  }
}

// =======================================================
// == Funktion zum berarbeiten der Events (Setzer VIEW) ==
// =============================================================================================================================
static void setzer_button_event_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e); // Button-Objekt bekommen

  if (event_code == LV_EVENT_CLICKED) { // Reagiere auf Klick-Event für normale Buttons
    lv_obj_t * label = lv_obj_get_child(obj, NULL); // Label des Buttons holen
    uintptr_t midi_note_ptr = (uintptr_t)lv_obj_get_user_data(obj); // MIDI Note als User Data holen
    byte midi_note = (byte)midi_note_ptr; // Zu byte umwandeln

    if (label) {
      Serial.print("Setzer Button '");
      Serial.print(lv_label_get_text(label)); // Button Beschriftung auslesen
      Serial.print("' wurde GEDRÜCKT, MIDI Note: ");
      Serial.println(midi_note); // MIDI Note ausgeben

      // MIDI Note senden (Note ON)
      sendNoteON(midi_note);
      // Hinweis: Für normale Buttons typischerweise Note ON beim Drücken,
      // und optional Note OFF beim Loslassen (LV_EVENT_RELEASED), falls gewünscht.

    } else {
      Serial.print("Setzer Button (ohne Label) wurde GEDRÜCKT, MIDI Note: ");
      Serial.println(midi_note); // MIDI Note ausgeben (Fallback)
      sendNoteON(midi_note); // MIDI Note senden (Fallback)
    }
  }
  // Hinweis: Für normale Buttons reagieren wir typischerweise auf LV_EVENT_CLICKED.
  // Eventuell auch auf LV_EVENT_PRESSED und LV_EVENT_RELEASED für genauere Steuerung.
}

// ======================================================
// == Funktion zum berarbeiten der Events (Setup VIEW) ==
// =============================================================================================================================
static void setup_button_event_handler(lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e); // Button-Objekt bekommen

    if (event_code == LV_EVENT_CLICKED) { // Reagiere auf Klick-Event für Setup Buttons
        lv_obj_t * label = lv_obj_get_child(obj, NULL); // Label des Buttons holen
        char *selected_organ = (char*)lv_obj_get_user_data(obj); // Orgelname aus User Data holen!

        if (label) {
            Serial.print("Orgel Auswahl Button '");
            Serial.print(lv_label_get_text(label)); // Button Beschriftung (Orgelname) auslesen
            Serial.print("' wurde GEDRÜCKT, Orgel Name: ");
            Serial.println(selected_organ); // Ausgewählten Orgelnamen ausgeben

            // **Aktualisiere die globalen Variablen `current_organ_name` und `current_buttonText`!**
            //current_organ_name = selected_organ; //  Speichere den ausgewählten Orgelnamen
            if (strcmp(current_organ_name, "Organ 1") == 0) {
                Serial.println("Orgel 0");
            } else if (strcmp(current_organ_name, "Organ 2") == 0) {
                Serial.println("Orgel 1");
            } else if (strcmp(current_organ_name, "Organ 3") == 0) {
                Serial.println("Orgel 2");
            }
            Serial.printf("  current_buttonText pointer nach Update: %p\n", current_buttonText); // DEBUG: Pointer ausgeben!

            // **Dynamische Auswahl des Button-Text-Arrays basierend auf current_organ_name (siehe oben):**
            if (strcmp(lv_label_get_text(label), "Organ 1") == 0) {
                sendNoteON(1);
                updateRegisterButtonLabels(buttonText); // Übergib organ1_buttonText für "Organ 1"
            } else if (strcmp(lv_label_get_text(label), "Organ 2") == 0) {
                sendNoteON(2);
                updateRegisterButtonLabels(buttonText1); // Übergib organ2_buttonText für "Organ 2"
            } else if (strcmp(lv_label_get_text(label), "Organ 3") == 0) {
                sendNoteON(3);
                updateRegisterButtonLabels(buttonText2); // Übergib organ3_buttonText für "Organ 3"
            }


        } else {
            Serial.println("Orgel Auswahl Button (ohne Label) wurde GEDRÜCKT"); // Fallback
        }
    }
}

// ====================================================================
// == Funktion zum Aktualisieren der Button-Labels im "Register" Tab ==
// =============================================================================================================================
void updateRegisterButtonLabels(char **buttonTextArray) {
    Serial.println("Funktion updateRegisterButtonLabels() aufgerufen.");

    for (int zeile = 0; zeile < 7; zeile++) {
        for (int spalte = 0; spalte < 6; spalte++) {
            lv_obj_t *btn = RegisterButtons[zeile + spalte * 7]; // Button aus dem RegisterButtons Array holen
            if (btn != NULL) {
                lv_obj_t * label = lv_obj_get_child(btn, NULL); // Label des Buttons holen
                if (label) {
                    int button_index = zeile + spalte * 7; // Index berechnen (wie bei Button-Erstellung)
                    // **JETZT buttonTextArray anstelle von current_buttonText verwenden:**
                    lv_label_set_text(label, buttonTextArray[button_index]); // **Beschriftung aus dem ÜBERGEBENEN Array setzen!**
                    Serial.print("Button (Zeile="); Serial.print(zeile); Serial.print(", Spalte="); Serial.print(spalte); Serial.print(") Label aktualisiert zu: '"); Serial.print(buttonTextArray[button_index]); Serial.println("'"); // Debug-Ausgabe anpassen
                } else {
                    Serial.println("FEHLER: Button Label ist NULL!"); // Fehlerfall: Label existiert nicht
                }
            } else {
                Serial.println("FEHLER: Button Objekt ist NULL!"); // Fehlerfall: Button existiert nicht
            }
        }
    }
    Serial.println("Aktualisierung der Button-Labels im 'Register' Tab abgeschlossen.");
}

// =========================================================================
// == Handler für MIDI Note ON Signale - Button EINschalten (unverändert) ==
// =============================================================================================================================
void handleMidiONSignal(byte note, byte velocity){
    Serial.print("Note ON Handler - Note:");
    Serial.println(note);
    updateRegisterButtonState(note, true); // Button Zustand auf ON setzen - Button EINschalten
}

// ==========================================================================
// == Handler für MIDI Note OFF Signale - Button AUSschalten (unverändert) ==
// =============================================================================================================================
void handleMidiOFFSignal(byte note, byte velocity){
    Serial.print("Note OFF Handler - Note:");
    Serial.println(note);
    updateRegisterButtonState(note, false); // Button Zustand auf OFF setzen - Button AUSschalten
}

// ===============================================================================
// == USBMIDI Event Handler Funktion für ESP32 USBMIDI Bibliothek (unverändert) ==
// =============================================================================================================================
void midi_in_handle(midiEventPacket_t &midi_packet_in) {
    uint8_t cable_num = MIDI_EP_HEADER_CN_GET(midi_packet_in.header);
    midi_code_index_number_t code_index_num = MIDI_EP_HEADER_CIN_GET(midi_packet_in.header);

    switch (code_index_num) {
        case MIDI_CIN_NOTE_ON:
            Serial.print("MIDI Note ON empfangen (ESP32 USBMIDI): Note=");
            Serial.println(midi_packet_in.byte2);
            handleMidiONSignal(midi_packet_in.byte2, midi_packet_in.byte3); // Aufruf der ON Signal Handler Funktion
            break;
        case MIDI_CIN_NOTE_OFF:
            Serial.print("MIDI Note OFF empfangen (ESP32 USBMIDI): Note=");
            Serial.println(midi_packet_in.byte2);
            handleMidiOFFSignal(midi_packet_in.byte2, midi_packet_in.byte3); // Aufruf der OFF Signal Handler Funktion
            break;
        // Hier könnten weitere MIDI Nachrichtentypen verarbeiten (Control Change, etc.)
    }
}

// =====================================================================
// == Zustand der Toggle Buttons anpassen bei Midi eingehenden Signal ==
// =============================================================================================================================
void updateRegisterButtonState(byte midiNote, bool noteOn) {
    for (int zeile = 0; zeile < 7; zeile++) {
        for (int spalte = 0; spalte < 6; spalte++) {
            lv_obj_t *btn;
            byte buttonMidiNote;
            int button_index = zeile + spalte * 7; // **Index für das Button-Array berechnen**

            // **JETZT ZUGRIFF AUF DAS BUTTON ARRAY: RegisterButtons!**
            btn = RegisterButtons[button_index];


            if (btn != NULL) { // Sicherstellen, dass der Button existiert
                buttonMidiNote = (byte)(uintptr_t)lv_obj_get_user_data(btn); // MIDI Note des Buttons holen
                if (buttonMidiNote == midiNote) { // MIDI Note des Buttons mit der empfangenen MIDI Note vergleichen
                    // **WICHTIG: Event Handler VOR Zustandsänderung entfernen!**
                    lv_obj_remove_event_cb(btn, button_event_handler);

                    if (noteOn) {
                        lv_obj_add_state(btn, LV_STATE_CHECKED); // Button aktivieren (Checked Zustand setzen)
                    } else {
                        lv_obj_clear_state(btn, LV_STATE_CHECKED); // Button deaktivieren (Checked Zustand entfernen)
                    }

                    // **Event Handler WIEDER HINZUFÜGEN, nachdem Zustand geändert wurde!**
                    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
                    return; // Button gefunden und Zustand geändert, Funktion verlassen
                }
            }
        }
    }
}