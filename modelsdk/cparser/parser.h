#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"



typedef enum xcb_auth_event_code_s {
  /*******************Login/Logout******************/
  // Request login 请求登录
  XCB_EVENT_LOGIN_REQUEST = 0x0001,
  // Login allowed 允许登录
  XCB_EVENT_LOGIN_ALLOWED = 0x0002,
  // Login rejected 登录被拒绝
  XCB_EVENT_LOGIN_REJECTED = 0x0003,
  // Request logout 登录退出
  XCB_EVENT_LOGOUT_REQUEST = 0x0004,
  // Undefined "login/logout" event! 未识别的登录或退出事件
  XCB_EVENT_LOGINOUT_UNDEFINED = 0x000f,

  /*****************Authorization*******************/
  // Request authorization 请求授权
  XCB_EVENT_AUTH_REQUEST = 0x1001,
  // Authorization allowed 允许授权
  XCB_EVENT_AUTH_ALLOWED = 0x1002,
  // Authorization rejected 授权被拒绝
  XCB_EVENT_AUTH_REJECTED = 0x1003,
  // Undefined "authorization" event! 未识别的授权事件
  XCB_EVENT_AUTH_UNDEFINED = 0x100f,

  /********create authentication file or not***********/
  // Success to create authentication file 鉴权文件生成成功
  XCB_EVENT_CREATE_FILE_SUCCESS = 0x2001,
  // Fail to create authentication file 鉴权文件生成失败
  XCB_EVENT_CREATE_FILE_FAIL = 0x2002,
  // Undefined "create authentication file" event! 未识别的鉴权文件生成事件
  XCB_EVENT_CREATE_FILE_UNDEFINED = 0x200f,

  /*****************query order************************/
  // Query order 查询订单
  XCB_EVENT_ORDER_QUERY = 0x3001,
  // Valid order status 订单状态有效
  XCB_EVENT_ORDER_VALID = 0x3002,
  // Invalid order status 订单状态无效
  XCB_EVENT_ORDER_INVALID = 0x3003,

  // Undefined event code! 未识别的事件码
  XCB_EVENT_CODE_UNDEFINED = 0xf000
} xcb_auth_event_code_t;



typedef struct {
  unsigned char size_pc_id;          // Bytes of PC ID
  unsigned char size_auth_code;      // Bytes of Authorization code
  unsigned char size_embed_id;       // Bytes of embedded ID
  unsigned char size_check_code;     // Bytes of check code
  unsigned char size_company;        // Bytes of company name
  unsigned char size_project;        // Bytes of project name
  unsigned short size_order_profile; // Bytes of order profile
  unsigned char type_embed_id;       // Type of embed id, 0x07-CPU ID
  xcb_auth_event_code_t event;
  unsigned char *pc_id;         // PC ID
  unsigned char *auth_code;     // Authorization code(bound with PC ID)
  unsigned char *embedded_id;   // Embedded ID
  unsigned char *check_code;    // Check code
  unsigned char *company;       // Company name
  unsigned char *project;       // Project name("order number" actually)
  unsigned char *order_profile; // order profiles
  unsigned int times_remaining; // Remaining authorization times
  unsigned int times_total;     // total authorization times
} xcb_auth_backend_data_t;

xcb_auth_backend_data_t parse_binary_data(const unsigned char *data, size_t length);
void xcb_auth_backend_data_free(xcb_auth_backend_data_t &data);



// example: "113c25c63289" --> {0x11,0x3C,0x25,0xC6,0x32,0x89}
int xcb_str2hex(const uint8_t *str, uint8_t *hex, uint32_t len_str,
                uint32_t len_hex);

// example: {0x11,0x3C,0x25,0xC6,0x32,0x89} --> "113c25c63289"
int xcb_hex2str(const uint8_t *hex, uint8_t *str, uint32_t len_hex,
                uint32_t len_str);
#endif