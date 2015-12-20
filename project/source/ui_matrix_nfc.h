#include "main.h"

const I8U ui_matrix_button[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */                 UI_DISPLAY_CAROUSEL_1,
	// 1: System
	/*UI_DISPLAY_DARK,*/                  UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/                UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/                    UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/               UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/        UI_DISPLAY_CAROUSEL_1,
	/*UI_DISPLAY_TRACKER_STEP,*/          UI_DISPLAY_CAROUSEL_1,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_CAROUSEL_1,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_CAROUSEL_1,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_CAROUSEL_1,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_HOME,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_CAROUSEL_1,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_CAROUSEL_1,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_HOME,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_HOME,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_HOME,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/                  UI_DISPLAY_HOME ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/                  UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_ROW,*/                  UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/                 UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/                    UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/                    UI_DISPLAY_HOME,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_CAROUSEL_2,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_HOME,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};

const I8U ui_matrix_finger_middle[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */                 UI_DISPLAY_HOME,
	// 1: System
	/*UI_DISPLAY_DARK,*/                  UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/                UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/                    UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/               UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/        UI_DISPLAY_TRACKER_UV_IDX,
	/*UI_DISPLAY_TRACKER_STEP,*/          UI_DISPLAY_TRACKER_STEP,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_TRACKER_SLEEP,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_TRACKER_DISTANCE,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_TRACKER_CALORIES,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_SMART_MESSAGE,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_SMART_DETAIL_NOTIF,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_SMART_PHONE_FINDER,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_SMART_INCOMING_CALL,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_SMART_INCOMING_MESSAGE,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_SMART_REMINDER,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_SMART_WEATHER,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_VITAL_HEART_RATE,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_SETTING_VER,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_STOPWATCH_START,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_STOPWATCH_RESULT,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/                  UI_DISPLAY_WORKOUT_WALKING ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/                  UI_DISPLAY_WORKOUT_RUNNING,
	/*UI_DISPLAY_WORKOUT_ROW,*/                  UI_DISPLAY_WORKOUT_ROW,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/                 UI_DISPLAY_WORKOUT_ELLIPTICAL,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/                 UI_DISPLAY_WORKOUT_STAIRS,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/                 UI_DISPLAY_WORKOUT_CYCLING,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/                 UI_DISPLAY_WORKOUT_AEROBIC,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/                 UI_DISPLAY_WORKOUT_PILOXING,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/                 UI_DISPLAY_WORKOUT_OTHERS,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/                    UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/                    UI_DISPLAY_WORKOUT_OUTDOOR,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_STOPWATCH_START,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_SMART_REMINDER,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};

const I8U ui_matrix_finger_left[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */ UI_DISPLAY_HOME,
	// 1: System
	/*UI_DISPLAY_DARK,*/UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/UI_DISPLAY_TRACKER_UV_IDX,
	/*UI_DISPLAY_TRACKER_STEP,*/UI_DISPLAY_TRACKER_STEP,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_TRACKER_SLEEP,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_TRACKER_DISTANCE,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_TRACKER_CALORIES,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_SMART_DETAIL_NOTIF,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_SMART_PHONE_FINDER,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_SMART_INCOMING_CALL,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_SMART_REMINDER,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_SMART_WEATHER,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_VITAL_HEART_RATE,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_HOME,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_HOME,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_HOME,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/UI_DISPLAY_HOME ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_ROW,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/UI_DISPLAY_HOME,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_WORKOUT_WALKING,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_SMART_WEATHER,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};

const I8U ui_matrix_finger_right[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */ UI_DISPLAY_HOME,
	// 1: System
	/*UI_DISPLAY_DARK,*/UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/UI_DISPLAY_TRACKER_UV_IDX,
	/*UI_DISPLAY_TRACKER_STEP,*/UI_DISPLAY_TRACKER_STEP,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_TRACKER_SLEEP,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_TRACKER_DISTANCE,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_TRACKER_CALORIES,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_SMART_DETAIL_NOTIF,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_SMART_DETAIL_NOTIF,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_SMART_PHONE_FINDER,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_SMART_DETAIL_NOTIF,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_SMART_REMINDER,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_SMART_WEATHER,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_VITAL_HEART_RATE,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_SETTING_VER,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_RESULT,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_STOPWATCH_RESULT,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/UI_DISPLAY_WORKOUT_INDOOR ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_ROW,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/UI_DISPLAY_WORKOUT_INDOOR,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/UI_DISPLAY_STOPWATCH_START,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/UI_DISPLAY_STOPWATCH_START,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_SMART_MESSAGE,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_SETTING_VER,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};

const I8U ui_matrix_swipe_left[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */ UI_DISPLAY_VITAL_HEART_RATE,
	// 1: System
	/*UI_DISPLAY_DARK,*/UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/UI_DISPLAY_TRACKER_UV_IDX,
	/*UI_DISPLAY_TRACKER_STEP,*/UI_DISPLAY_TRACKER_CALORIES,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_TRACKER_STEP,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_TRACKER_DISTANCE,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_SMART_MESSAGE,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_SMART_PHONE_FINDER,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_SMART_INCOMING_CALL,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_SMART_INCOMING_MESSAGE,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_SMART_WEATHER,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_VITAL_SKIN_TEMP,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_TRACKER_SLEEP,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_SETTING_VER,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_STOPWATCH_START,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_STOPWATCH_RESULT,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/UI_DISPLAY_WORKOUT_RUNNING ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/UI_DISPLAY_WORKOUT_ROW,
	/*UI_DISPLAY_WORKOUT_ROW,*/UI_DISPLAY_WORKOUT_ELLIPTICAL,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/UI_DISPLAY_WORKOUT_STAIRS,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/UI_DISPLAY_WORKOUT_CYCLING,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/UI_DISPLAY_WORKOUT_AEROBIC,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/UI_DISPLAY_WORKOUT_PILOXING,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/UI_DISPLAY_WORKOUT_OTHERS,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/UI_DISPLAY_WORKOUT_WALKING,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/UI_DISPLAY_WORKOUT_OUTDOOR,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/UI_DISPLAY_WORKOUT_INDOOR,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_CAROUSEL_2,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_CAROUSEL_1,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};


const I8U ui_matrix_swipe_right[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */ UI_DISPLAY_TRACKER_DISTANCE,
	// 1: System
	/*UI_DISPLAY_DARK,*/UI_DISPLAY_DARK,
	/*UI_DISPLAY_CLING,*/UI_DISPLAY_CLING,
	/*UI_DISPLAY_OTA,*/UI_DISPLAY_OTA,
	/*UI_DISPLAY_PAIRING,*/UI_DISPLAY_PAIRING,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/UI_DISPLAY_TRACKER_UV_IDX,
	/*UI_DISPLAY_TRACKER_STEP,*/UI_DISPLAY_TRACKER_SLEEP,
	/*UI_DISPLAY_TRACKER_SLEEP,*/UI_DISPLAY_VITAL_SKIN_TEMP,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/UI_DISPLAY_TRACKER_CALORIES,
	/*UI_DISPLAY_TRACKER_CALORIES,*/UI_DISPLAY_TRACKER_STEP,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/UI_DISPLAY_SMART_MESSAGE,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/UI_DISPLAY_SMART_APP_NOTIF,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/UI_DISPLAY_SMART_PHONE_FINDER,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/UI_DISPLAY_SMART_INCOMING_CALL,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/UI_DISPLAY_SMART_INCOMING_MESSAGE,
	/*UI_DISPLAY_SMART_REMINDER,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_SMART_WEATHER,*/UI_DISPLAY_SMART_WEATHER,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/UI_DISPLAY_HOME,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/UI_DISPLAY_VITAL_HEART_RATE,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/UI_DISPLAY_SETTING_VER,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/ UI_DISPLAY_STOPWATCH_START,
	/*UI_DISPLAY_STOPWATCH_STOP,*/UI_DISPLAY_STOPWATCH_CALORIES,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/UI_DISPLAY_STOPWATCH_STOP,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/UI_DISPLAY_STOPWATCH_HEARTRATE,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/UI_DISPLAY_STOPWATCH_RESULT,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/UI_DISPLAY_WORKOUT_OTHERS ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/UI_DISPLAY_WORKOUT_WALKING,
	/*UI_DISPLAY_WORKOUT_ROW,*/UI_DISPLAY_WORKOUT_RUNNING,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/UI_DISPLAY_WORKOUT_ROW,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/UI_DISPLAY_WORKOUT_ELLIPTICAL,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/UI_DISPLAY_WORKOUT_STAIRS,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/UI_DISPLAY_WORKOUT_CYCLING,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/UI_DISPLAY_WORKOUT_AEROBIC,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/UI_DISPLAY_WORKOUT_PILOXING,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/UI_DISPLAY_WORKOUT_OUTDOOR,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/UI_DISPLAY_WORKOUT_INDOOR,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/UI_DISPLAY_CAROUSEL_2,
	/*UI_DISPLAY_CAROUSEL_2,*/UI_DISPLAY_CAROUSEL_1,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/UI_DISPLAY_PREVIOUS,
};

//
// UI FRAME GESTURE (UFG) constrains
//
#define UFG_NONE                         0x00
#define UFG_SWIPE_PANNING                0x01
#define UFG_FINGER_IRIS                  0x02
#define UFG_BUTTON_SINGLE                0x04
#define UFG_BUTTON_HOLD                  0x08

#define S_F_BS_BH (UFG_SWIPE_PANNING | UFG_FINGER_IRIS | UFG_BUTTON_SINGLE | UFG_BUTTON_HOLD)
#define F_BS (UFG_FINGER_IRIS | UFG_BUTTON_SINGLE)
#define F_BS_BH (UFG_FINGER_IRIS | UFG_BUTTON_SINGLE | UFG_BUTTON_HOLD)
#define N_F (UFG_NONE | UFG_FINGER_IRIS)
#define S_F_BS (UFG_SWIPE_PANNING | UFG_FINGER_IRIS | UFG_BUTTON_SINGLE)
#define S_F (UFG_SWIPE_PANNING | UFG_FINGER_IRIS)

const I8U ui_gesture_constrain[UI_DISPLAY_MAXIMUM] = {
	// 0: Home
	/* UI_DISPLAY_HOME */                        S_F_BS_BH,
	// 1: System
	/*UI_DISPLAY_DARK,*/                         S_F_BS_BH,
	/*UI_DISPLAY_CLING,*/                        S_F_BS_BH,
	/*UI_DISPLAY_OTA,*/                          N_F,
	/*UI_DISPLAY_PAIRING,*/                      N_F,
	// 2: a set of the typical use cases
	/*UI_DISPLAY_TRACKER_UV_IDX,*/               S_F_BS_BH,
	/*UI_DISPLAY_TRACKER_STEP,*/                 S_F_BS_BH,
	/*UI_DISPLAY_TRACKER_SLEEP,*/                S_F_BS_BH,
	/*UI_DISPLAY_TRACKER_DISTANCE,*/             S_F_BS_BH,
	/*UI_DISPLAY_TRACKER_CALORIES,*/             S_F_BS_BH,
	// 3: a set of battery stats
	/*UI_DISPLAY_SMART_MESSAGE,*/                F_BS_BH,
	/*UI_DISPLAY_SMART_APP_NOTIF,*/              S_F_BS_BH,
	/*UI_DISPLAY_SMART_DETAIL_NOTIF,*/           S_F_BS_BH,
	/*UI_DISPLAY_SMART_PHONE_FINDER,*/           S_F_BS_BH,
	/*UI_DISPLAY_SMART_INCOMING_CALL,*/          S_F_BS,
	/*UI_DISPLAY_SMART_INCOMING_MESSAGE,*/       S_F_BS,
	/*UI_DISPLAY_SMART_REMINDER,*/               S_F_BS_BH,
	/*UI_DISPLAY_SMART_WEATHER,*/                S_F_BS_BH,
	// 4: a set of VITAL
	/*UI_DISPLAY_VITAL_HEART_RATE,*/             S_F_BS_BH,
	/*UI_DISPLAY_VITAL_SKIN_TEMP,*/              S_F_BS_BH,
	// 5: Special back panel mode
	/*UI_DISPLAY_SETTING_VER,*/                  F_BS,
	// 6: Confirmation messages
	/*UI_DISPLAY_STOPWATCH_START,*/              N_F,
	/*UI_DISPLAY_STOPWATCH_STOP,*/               S_F,
	/*UI_DISPLAY_STOPWATCH_HEARTRATE,*/          S_F,
	/*UI_DISPLAY_STOPWATCH_CALORIES,*/           S_F,
	/*UI_DISPLAY_STOPWATCH_RESULT,*/             N_F,
	// 7: Info menu
	/*UI_DISPLAY_WORKOUT_WALKING,*/              S_F_BS ,
	/*UI_DISPLAY_WORKOUT_RUNNING,*/              S_F_BS,
	/*UI_DISPLAY_WORKOUT_ROW,*/                  S_F_BS,
	/*UI_DISPLAY_WORKOUT_ELLIPTICAL,*/           S_F_BS,
	/*UI_DISPLAY_WORKOUT_STAIRS,*/               S_F_BS,
	/*UI_DISPLAY_WORKOUT_CYCLING,*/              S_F_BS,
	/*UI_DISPLAY_WORKOUT_AEROBIC,*/              S_F_BS,
	/*UI_DISPLAY_WORKOUT_PILOXING,*/             S_F_BS,
	/*UI_DISPLAY_WORKOUT_OTHERS,*/               S_F_BS,
	/*UI_DISPLAY_WORKOUT_INDOOR,*/               S_F_BS,
	/*UI_DISPLAY_WORKOUT_OUTDOOR,*/              S_F_BS,
	// 9: CAROUSEL
	/*UI_DISPLAY_CAROUSEL_1,*/                   S_F_BS,
	/*UI_DISPLAY_CAROUSEL_2,*/                   S_F_BS,
	// 10: PREVIOUS
	/*UI_DISPLAY_PREVIOUS*/                      S_F_BS_BH,
};

