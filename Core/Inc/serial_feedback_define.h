//
// Created by TonyGuo on 2022/11/17.
//

#ifndef FATHOMETER_MAINFRAME_SERIAL_FEEDBACK_DEFINE_H
#define FATHOMETER_MAINFRAME_SERIAL_FEEDBACK_DEFINE_H

#define SERIAL_FEEDBACK_INFO_SYSTEM_INIT                                "[Info] Initializing system components...\r\n"
#define SERIAL_FEEDBACK_INFO_GET_FIRMWARE_VERSION                       "[Info] Firmware Version: %s\r\n"
#define SERIAL_FEEDBACK_ERROR_IPP_TRIGGERED                             "\r\n[Error] Intellectual Property Protection mechanism triggered - Illegal Operation\r\n"
#define SERIAL_FEEDBACK_INFO_IPP_SET                                    "[Info] Factory Mode: Intellectual Property Protection is now set\r\n"
#define SERIAL_FEEDBACK_WARNING_IPP_LEGAL_NOTICE                        "[Warning] It is illegal to reverse engineer this device or software!\r\n\r\n"
#define SERIAL_FEEDBACK_INFO_SHUTDOWN_SERIAL                            "[Info] Stopped responding to serial commands\r\n"
#define SERIAL_FEEDBACK_INFO_SHUTDOWN_HEARTBEAT                         "[Info] Status feedback disabled\r\n"
#define SERIAL_FEEDBACK_INFO_CONTACT_INFO                               "[Info] Contact the manufacturer for support and service\r\n"
#define SERIAL_FEEDBACK_INFO_GET_BAUD_RATE                              "[Info] Baud rate: 115200\r\n"
#define SERIAL_FEEDBACK_INFO_ADC_INIT                                   "[Info] Power supply status monitor initialized\r\n[Info] Main Voltage:%.2fV, System Voltage:%.2fV\r\n"
#define SERIAL_FEEDBACK_ERROR_GET_SERIAL_NUMBER_NOT_SET                 "R*E0\r\n"
#define SERIAL_FEEDBACK_ERROR_COMMAND_OVERFLOW                          "R*E1\r\n"
#define SERIAL_FEEDBACK_ERROR_COMMAND_NOT_FOUND                         "R*E2\r\n"
#define SERIAL_FEEDBACK_ERROR_RTC_INVALID                               "R*E3\r\n"
#define SERIAL_FEEDBACK_ERROR_TASK_TIME_SYNC_FAIL                       "R*E4\r\n"
#define SERIAL_FEEDBACK_ERROR_TASK_BUSY                                 "R*E5\r\n"
#define SERIAL_FEEDBACK_ERROR_ILLEGAL_SERIAL_NUMBER                     "R*E6\r\n"
#define SERIAL_FEEDBACK_ERROR_FLASH__POINTER                            "R*E7\r\n"
#define SERIAL_FEEDBACK_ERROR_CRC_ILLEGAL                               "R*E8\r\n"
#define SERIAL_FEEDBACK_ERROR_CRC_FAIL                                  "R*E9\r\n"
#define SERIAL_FEEDBACK_INFO_TASK_CREATE                                "R*T%d\r\n"
#define SERIAL_FEEDBACK_INFO_GET_SERIAL_NUMBER                          "R*s%s\r\n"
#define SERIAL_FEEDBACK_INFO_FLASH_ERASE                                "R*X\r\n"
#define SERIAL_FEEDBACK_INFO_DEBUG_ON                                   "R*Start Debug Mode\r\n"
#define SERIAL_FEEDBACK_INFO_DEBUG_OFF                                  "R*Stop Debug Mode\r\n"
#define SERIAL_FEEDBACK_INFO_CRC_PASS                                   "R*C20%02d%02d%02d%02d%02d%02d\r\n"
#define SERIAL_FEEDBACK_INFO_QUERY_STATUS                               "R*I%08lX,%d,%.2f,%.2f\r\n"
#define SERIAL_FEEDBACK_INFO_FETCH_DATA                                 "R*F20%02d%02d%02d%02d%02d%02d,%d,%08lX,%08lX,%08lX,%08lX,%08lX,%08lX\r\n"
#define SERIAL_FEEDBACK_INFO_SYSTEM_RESTART_PROCEED                     "R*R\r\n"
#define SERIAL_FEEDBACK_INFO_GET_SERIAL_NUMBER_INITIAL                  "[Info] Serial Number: %s\r\n"
#define SERIAL_FEEDBACK_INFO_SET_SERIAL_NUMBER                          "R*S%c%c%c%c%c%c%c%c\r\n"
#define SERIAL_FEEDBACK_INFO_RTC_INIT                                   "[Info] RTC initialized, current datetime: %02d/%02d/%02d %02d:%02d:%02d\r\n"
#define SERIAL_FEEDBACK_INFO_GET_RTC_TIME                               "R*d%02d%02d%02d%02d%02d%02d\r\n"
#define SERIAL_FEEDBACK_INFO_SWITCH_OPERATING_MODE                      "[Info] Now operating at Mode %c\r\n"
#define SERIAL_FEEDBACK_ERROR_OPERATING_MODE_INVALID                    "[Error] Operating Mode invalid\r\n"
#define SERIAL_FEEDBACK_INFO_FLASH_INIT                                 "[Info] Flash Initialized\r\n"
#define SERIAL_FEEDBACK_INFO_RECORDS_ALERT                              "[Info] %d record(s) waiting to be transferred\r\n"
#define SERIAL_FEEDBACK_INFO_FLASH_D_PTR_START_INIT                     "[Info] Flash data pointer start reset to default value\r\n"
#define SERIAL_FEEDBACK_INFO_FLASH_D_PTR_END_INIT                       "[Info] Flash data pointer end reset to default value\r\n"

#endif //FATHOMETER_MAINFRAME_SERIAL_FEEDBACK_DEFINE_H
