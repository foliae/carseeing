#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xcb_auth_backend.h"
#include "parser.h"



void xcb_auth_backend_data_free(xcb_auth_backend_data_t &data)
{
  if (data.pc_id != 0)
  {
    delete[] data.pc_id;
    data.pc_id = 0;
  }
  if (data.auth_code != 0)
  {
    delete[] data.auth_code;
    data.auth_code = 0;
  }
  if (data.embedded_id != 0)
  {
    delete[] data.embedded_id;
    data.embedded_id = 0;
  }
  if (data.check_code != 0)
  {
    delete[] data.check_code;
    data.check_code = 0;
  }
  if (data.company != 0)
  {
    delete[] data.company;
    data.company = 0;
  }
  if (data.project != 0)
  {
    delete[] data.project;
    data.project = 0;
  }
  if (data.order_profile != 0)
  {
    delete[] data.order_profile;
    data.order_profile = 0;
  }
  data.size_pc_id         = 0;
  data.size_auth_code     = 0;
  data.size_embed_id      = 0;
  data.size_check_code    = 0;
  data.size_company       = 0;
  data.size_project       = 0;
  data.size_order_profile = 0;
  data.times_remaining    = 0;
  data.times_total        = 0;
  data.event              = XCB_EVENT_CODE_UNDEFINED;
}

xcb_auth_backend_data_t parse_binary_data(const unsigned char *binary_data, size_t data_size) {
  xcb_auth_backend_data_t data;
  xcb_auth_backend_data_init(data);

  if (data_size % AES_BLOCKLEN != 0)
  {
    return data;
  }

  decrypt_data_from_pc_to_backend(binary_data, data_size, data);


  data.pc_id[data.size_pc_id] = '\0';
  data.auth_code[data.size_auth_code] = '\0';

  if (data.event == XCB_EVENT_LOGIN_REQUEST || data.event == XCB_EVENT_LOGOUT_REQUEST)
  {
  }
  else if (data.event == XCB_EVENT_AUTH_REQUEST || data.event == XCB_EVENT_CREATE_FILE_SUCCESS ||
           data.event == XCB_EVENT_CREATE_FILE_FAIL)
  {
    data.project[data.size_project] = '\0';

    if (data.type_embed_id == 0x07)
    {
      uint8_t *embed_id = new uint8_t[data.size_embed_id * 2 + 1];
      xcb_hex2str(data.embedded_id, embed_id, data.size_embed_id, data.size_embed_id * 2 + 1);
      memcpy(data.embedded_id, embed_id, data.size_embed_id*2 + 1 );
      data.size_embed_id = data.size_embed_id * 2;
      delete[] embed_id;
    }
    else
    {
      data.embedded_id[data.size_embed_id] = '\0';
    }
    uint8_t *check_code = new uint8_t[data.size_check_code * 2 + 1];
    xcb_hex2str(data.check_code, check_code, data.size_check_code, data.size_check_code * 2 + 1);
    memcpy(data.check_code, check_code, data.size_check_code*2 + 1 );
    data.size_check_code = data.size_check_code * 2;
    delete[] check_code;
  }
  else if (data.event == XCB_EVENT_ORDER_QUERY)
  {
    data.project[data.size_project] = '\0';
  }

  //xcb_auth_backend_data_free(data);
  return data;
}


int test()
{
  std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
      std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
  auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  unsigned long long timestamp = tmp.count();

  XCB_RNG rng(timestamp);

  uint8_t key_pc[32] = {0x4a, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0xc5, 0xae,
                        0xf0, 0x85, 0x7d, 0xaa, 0x81, 0x1f, 0xf5, 0x2c, 0x17, 0x3b, 0x9e,
                        0x08, 0xd3, 0x2d, 0x9a, 0x30, 0xa7, 0x33, 0x14, 0xdf, 0xf4};

  uint8_t key_backend[32] = {0xca, 0x5d, 0xeb, 0x14, 0x15, 0xca, 0x70, 0xbe, 0x2b, 0xc5, 0xae,
                             0xf0, 0x87, 0x7d, 0x4a, 0x82, 0x17, 0x77, 0x2c, 0x57, 0x7b, 0x9e,
                             0x08, 0xd5, 0x8d, 0x3e, 0x50, 0xa7, 0x53, 0x1c, 0xd5, 0xfc};

  uint8_t key_embed[32] = {0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x2b, 0xc5,
                           0xae, 0xf0, 0x87, 0x7d, 0x4a, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
                           0xad, 0x2b, 0x41, 0xd5, 0x8d, 0x3e, 0x50, 0xa7, 0x53, 0x1c};

  uint16_t event_code = 0x1001; // Request authorization 请求授权

  const char *pc_id   = "WD-WCC6Y4PLUK9Y"; // PC硬件ID，最多50个字节
  uint8_t bytes_pc_id = sizeof(pc_id);

  const char *pc_auth_code = "qwer2019"; // PC授权码(密码), 最多10个字节
  uint8_t bytes_pc_auth    = sizeof(pc_auth_code);

  uint8_t embed_data_mode = 0x07; //嵌入式端数据模式, 0x07-CPU ID(当前只用这种模式)
  uint32_t check_code     = 0xa5138c; //嵌入式端校验码

  const char *embed_id = "3c0f010b6541cc2000006c0000000000"; //嵌入式端CPU ID，32字节
  uint8_t embed_id_hex[16];
  uint8_t bytes_embed_id = 16;
  xcb_str2hex((const uint8_t *)embed_id, embed_id_hex, sizeof(embed_id),
              16); //转成对应的16字节字符

  //先对嵌入式端数据加密
  uint16_t bytes_embed_data = 1 /* 1字节随机数 */ + 1 /* 1字节嵌入式端数据模式 */ +
      1 /* 1字节随机数 */ + 4 /* 4字节校验码 */ + 1 /* 1字节随机数 */ +
      (1 /* 1字节保存CPUID字节数 */ + bytes_embed_id /* CPU ID字节数 */);
  bytes_embed_data        = (bytes_embed_data / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
  uint8_t *data_embed_raw = new uint8_t[bytes_embed_data];
  uint8_t *data_embed_enc = new uint8_t[bytes_embed_data];
  for (unsigned short i = 0; i < bytes_embed_data; i++)
  {
    data_embed_raw[i] = (unsigned char)(rng.uniform(0, 256));
  }

  int ipos_embed_data_mode = 1; // embed_data_mode的保存位置
  int ipos_check_code      = 3; // check_code保存位置
  int ipos_embed_id_bytes  = 8; //
  int ipos_embed_id        = 9;

  data_embed_raw[ipos_embed_data_mode] = embed_data_mode;
  memcpy(data_embed_raw + ipos_check_code, &check_code, 4);
  data_embed_raw[ipos_embed_id_bytes] = bytes_embed_id;
  memcpy(data_embed_raw + ipos_embed_id, embed_id_hex, bytes_embed_id);
  aes_cbc_encrypt(data_embed_enc, data_embed_raw, bytes_embed_data, key_embed, 32, NULL);

  char *data_embed_raw_str = new char[bytes_embed_data * 2 + 1];
  char *data_embed_enc_str = new char[bytes_embed_data * 2 + 1];
  memset(data_embed_raw_str, 0, bytes_embed_data * 2 + 1);
  memset(data_embed_enc_str, 0, bytes_embed_data * 2 + 1);
  xcb_hex2str(data_embed_raw, (uint8_t *)data_embed_raw_str, bytes_embed_data,
              bytes_embed_data * 2 + 1);
  xcb_hex2str(data_embed_enc, (uint8_t *)data_embed_enc_str, bytes_embed_data,
              bytes_embed_data * 2 + 1);

  //加密PC端数据+加密的嵌入式端数据
  unsigned short size_data = 1 + // 1字节随机数
      2 +                        // 2字节PC到后台数据
      1 +                        // 1字节随机数
      2 +                        // 2字节事件码
      1 +                        // 1字节随机数
      1 +                        // 1字节保存PC ID字节数
      50 +                       // 50字节保存PC ID
      1 +                        // 1字节随机数
      1 +                        // 1字节保存PC授权码字节数
      10 +                       // 10字节保存PC授权码
      1 +                        // 1字节随机数
      2 +                        // 2字节保存加密后的嵌入式端数据字节数
      1 +                        // 1字节随机数
      bytes_embed_data;          //加密后的嵌入式端数据
  size_data               = (size_data / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
  unsigned char *data_raw = new unsigned char[size_data];
  for (unsigned short i = 0; i < size_data; i++)
  {
    data_raw[i] = (unsigned char)(rng.uniform(0, 256));
  }

  int ipos_size_data       = 1;
  int ipos_event_code      = 4;
  int ipos_pc_id_bytes     = 7;
  int ipos_pc_id           = 8;
  int ipos_auth_code_bytes = 59;
  int ipos_auth_code       = 60;
  int ipos_embed_data_size = 71;
  int ipos_embed_data      = 74;

  memcpy(data_raw + ipos_size_data, &size_data, sizeof(unsigned short));
  memcpy(data_raw + ipos_event_code, &event_code, sizeof(unsigned short));
  *(data_raw + ipos_pc_id_bytes) = bytes_pc_id;
  memcpy(data_raw + ipos_pc_id, pc_id, bytes_pc_id);
  *(data_raw + ipos_auth_code_bytes) = bytes_pc_auth;
  memcpy(data_raw + ipos_auth_code, pc_auth_code, bytes_pc_auth);

  memcpy(data_raw + ipos_embed_data_size, &bytes_embed_data, sizeof(unsigned short));
  memcpy(data_raw + ipos_embed_data, data_embed_enc, bytes_embed_data);

  unsigned char *data_encrypt = new unsigned char[size_data];
  aes_cbc_encrypt(data_encrypt, data_raw, size_data, key_pc, 32, NULL);

  delete[] data_raw;
  delete[] data_encrypt;
  delete[] data_embed_raw;
  delete[] data_embed_enc;

  return 0;
}


// 0 - encrypt binary file with text file（服务器将待回传的txt文件加密成二进制文件再回传）
// 1 - decrypt binary file into text file（服务器将从网络接收的加密二进制文件解密成txt文件）
#define FUNC 1

#if FUNC == 1

// decrypt binary file into text file
int test_main(int argc, char *argv[])
{
  char binaryName[500], textName[500];
  strcpy(binaryName, "./xcb_data.bin");
  strcpy(textName, "./xcb_data.txt");
  for (int i = 1; i < argc;)
  {
    if (!strcmp(argv[i], "-bin_path"))
    {
      strcpy(binaryName, argv[i + 1]);
      i += 2;
    }
    else if (!strcmp(argv[i], "-txt_path"))
    {
      strcpy(textName, argv[i + 1]);
      i += 2;
    }
    else
    {
      i++;
    }
  }

  xcb_auth_backend_data_t data;
  xcb_auth_backend_data_init(data);

  FILE *fbin = fopen(binaryName, "rb");
  if (!fbin)
  {
    printf("Fail to read: \"%s\"\n", binaryName);
    FILE *ftxt = fopen(textName, "wt");
    fclose(ftxt);
    xcb_auth_backend_data_free(data);

    return -1;
  }
  fseek(fbin, 0L, SEEK_END);
  int data_size = ftell(fbin);
  if (data_size % AES_BLOCKLEN != 0)
  {
    fclose(fbin);
    printf("Incorrect file length: %d\n", data_size);
    FILE *ftxt = fopen(textName, "wt");
    fclose(ftxt);
    xcb_auth_backend_data_free(data);

    return -1;
  }
  fseek(fbin, 0L, SEEK_SET);

  unsigned char *data_encrypt = new unsigned char[data_size];
  fread(data_encrypt, 1, data_size, fbin);
  fclose(fbin);

  decrypt_data_from_pc_to_backend(data_encrypt, data_size, data);
  delete[] data_encrypt;

  FILE *ftxt = fopen(textName, "wt");

  fprintf(ftxt, "event_code: %u\n", data.event);
  data.pc_id[data.size_pc_id] = '\0';
  fprintf(ftxt, "pc_id: %s\n", data.pc_id);
  data.auth_code[data.size_auth_code] = '\0';
  fprintf(ftxt, "auth_code: %s\n", data.auth_code);

  if (data.event == XCB_EVENT_LOGIN_REQUEST || data.event == XCB_EVENT_LOGOUT_REQUEST)
  {
  }
  else if (data.event == XCB_EVENT_AUTH_REQUEST || data.event == XCB_EVENT_CREATE_FILE_SUCCESS ||
           data.event == XCB_EVENT_CREATE_FILE_FAIL)
  {
    data.project[data.size_project] = '\0';
    fprintf(ftxt, "order_num: %s\n", data.project);

    if (data.type_embed_id == 0x07)
    {
      uint8_t *embed_id = new uint8_t[data.size_embed_id * 2 + 1];
      xcb_hex2str(data.embedded_id, embed_id, data.size_embed_id, data.size_embed_id * 2 + 1);
      fprintf(ftxt, "embed_id: %s\n", embed_id);
      delete[] embed_id;
    }
    else
    {
      data.embedded_id[data.size_embed_id] = '\0';
      fprintf(ftxt, "embed_id: %s\n", data.embedded_id);
    }
    uint8_t *check_code = new uint8_t[data.size_check_code * 2 + 1];
    xcb_hex2str(data.check_code, check_code, data.size_check_code, data.size_check_code * 2 + 1);
    fprintf(ftxt, "check_code: %s\n", check_code);
    delete[] check_code;
  }
  else if (data.event == XCB_EVENT_ORDER_QUERY)
  {
    data.project[data.size_project] = '\0';
    fprintf(ftxt, "order_num: %s\n", data.project);
  }

  fclose(ftxt);
  xcb_auth_backend_data_free(data);

  return 0;
}

#else

char *UTF82Char(const char *szU8)
{
  int wcsLen         = MultiByteToWideChar(CP_UTF8, NULL, szU8, (int)strlen(szU8), NULL, 0);
  wchar_t *wszString = new wchar_t[wcsLen + 1];
  MultiByteToWideChar(CP_UTF8, NULL, szU8, (int)strlen(szU8), wszString, wcsLen);
  wszString[wcsLen] = '\0';
  int len = WideCharToMultiByte(CP_ACP, 0, wszString, (int)wcslen(wszString), NULL, 0, NULL, NULL);
  char *c = new char[len + 1];
  WideCharToMultiByte(CP_ACP, 0, wszString, (int)wcslen(wszString), c, len, NULL, NULL);
  c[len] = '\0';
  delete[] wszString;
  return c;
}
// encrypt binary file with text file
int main(int argc, char *argv[])
{
  char binaryName[500], textName[500];
  strcpy(binaryName, "./xcb_data.bin");
  strcpy(textName, "./xcb_data.txt");
  for (int i = 1; i < argc;)
  {
    if (!strcmp(argv[i], "-bin_path"))
    {
      strcpy(binaryName, argv[i + 1]);
      i += 2;
    }
    else if (!strcmp(argv[i], "-txt_path"))
    {
      strcpy(textName, argv[i + 1]);
      i += 2;
    }
    else
    {
      i++;
    }
  }

  xcb_auth_backend_data_t data;
  xcb_auth_backend_data_init(data);

  FILE *ftxt = fopen(textName, "rt");
  if (!ftxt)
  {
    printf("Fail to read: \"%s\"\n", textName);
    FILE *fbin = fopen(binaryName, "wb");
    fclose(fbin);
    xcb_auth_backend_data_free(data);

    return -1;
  }

  int flag;
  unsigned int tmpInt;
  char tmpchar[200];
  flag = fscanf(ftxt, "%s", tmpchar); //"event_code:"
  if (flag <= 0 || strcmp(tmpchar, "event_code:"))
  {
    printf("\"%s\" is empty or format is wrong\n", textName);
    FILE *fbin = fopen(binaryName, "wb");
    fclose(fbin);
    xcb_auth_backend_data_free(data);

    return -1;
  }

  flag       = fscanf(ftxt, "%u", &tmpInt);
  data.event = (xcb_auth_event_code_t)tmpInt;
  if (data.event == XCB_EVENT_LOGIN_ALLOWED || data.event == XCB_EVENT_LOGIN_REJECTED)
  {
    flag                = fscanf(ftxt, "%s", tmpchar);    //"pc_id:"
    flag                = fscanf(ftxt, "%s", data.pc_id); // pc_id
    data.size_pc_id     = strlen((const char *)data.pc_id);
    flag                = fscanf(ftxt, "%s", tmpchar);        //"auth_code:"
    flag                = fscanf(ftxt, "%s", data.auth_code); // auth_code
    data.size_auth_code = strlen((const char *)data.auth_code);
  }
  else if (data.event == XCB_EVENT_AUTH_ALLOWED || data.event == XCB_EVENT_AUTH_REJECTED)
  {
    flag                = fscanf(ftxt, "%s", tmpchar);    //"pc_id:"
    flag                = fscanf(ftxt, "%s", data.pc_id); // pc_id
    data.size_pc_id     = strlen((const char *)data.pc_id);
    flag                = fscanf(ftxt, "%s", tmpchar);        //"auth_code:"
    flag                = fscanf(ftxt, "%s", data.auth_code); // auth_code
    data.size_auth_code = strlen((const char *)data.auth_code);

    flag              = fscanf(ftxt, "%s", tmpchar);      //"company:"
    flag              = fscanf(ftxt, "%s", data.company); // company
    data.size_company = strlen((const char *)data.company);
    flag              = fscanf(ftxt, "%s", tmpchar);      //"project:"
    flag              = fscanf(ftxt, "%s", data.project); // project
    data.size_project = strlen((const char *)data.project);

    flag = fscanf(ftxt, "%s", tmpchar);                 //"remaining_times:"
    flag = fscanf(ftxt, "%u", &(data.times_remaining)); // remaining_times

    flag  = fscanf(ftxt, "%s", tmpchar); //"embed_id:"
    flag  = fscanf(ftxt, "%s", tmpchar); // embed_id
    int n = strlen(tmpchar);
    if (n == 32)
    {
      uint8_t *embed_id = new uint8_t[16];
      xcb_str2hex((const uint8_t *)tmpchar, embed_id, 32, 16);
      memcpy(data.embedded_id, embed_id, 16);
      data.size_embed_id = 16;
      delete[] embed_id;
      data.type_embed_id = 0x07;
    }
    else
    {
      memcpy(data.embedded_id, tmpchar, n);
      data.size_embed_id = n;
      data.type_embed_id = 0x08;
    }

    flag = fscanf(ftxt, "%s", tmpchar); //"check_code:"
    flag = fscanf(ftxt, "%s", tmpchar); // check_code
    n    = strlen(tmpchar);
    if (n == 8)
    {
      xcb_str2hex((const uint8_t *)tmpchar, data.check_code, 8, 4);
      data.size_check_code = 4;
    }
    else
    {
      memcpy(data.check_code, tmpchar, n);
      data.size_check_code = n;
    }
  }
  else if (data.event == XCB_EVENT_ORDER_INVALID)
  {
    flag                = fscanf(ftxt, "%s", tmpchar);    //"pc_id:"
    flag                = fscanf(ftxt, "%s", data.pc_id); // pc_id
    data.size_pc_id     = strlen((const char *)data.pc_id);
    flag                = fscanf(ftxt, "%s", tmpchar);        //"auth_code:"
    flag                = fscanf(ftxt, "%s", data.auth_code); // auth_code
    data.size_auth_code = strlen((const char *)data.auth_code);
    flag                = fscanf(ftxt, "%s", tmpchar);      //"order_num:"
    flag                = fscanf(ftxt, "%s", data.project); // order_num
    data.size_project   = strlen((const char *)data.project);
  }
  else if (data.event == XCB_EVENT_ORDER_VALID)
  {
    flag                = fscanf(ftxt, "%s", tmpchar);    //"pc_id:"
    flag                = fscanf(ftxt, "%s", data.pc_id); // pc_id
    data.size_pc_id     = strlen((const char *)data.pc_id);
    flag                = fscanf(ftxt, "%s", tmpchar);        //"auth_code:"
    flag                = fscanf(ftxt, "%s", data.auth_code); // auth_code
    data.size_auth_code = strlen((const char *)data.auth_code);
    flag                = fscanf(ftxt, "%s", tmpchar);      //"order_num:"
    flag                = fscanf(ftxt, "%s", data.project); // order_num
    data.size_project   = strlen((const char *)data.project);

    flag = fscanf(ftxt, "%s", tmpchar);                 //"total_times:"
    flag = fscanf(ftxt, "%u", &(data.times_total));     // total_times
    flag = fscanf(ftxt, "%s", tmpchar);                 //"remaining_times:"
    flag = fscanf(ftxt, "%u", &(data.times_remaining)); // remaining_times

    flag                    = fscanf(ftxt, "%s", tmpchar);            //"order_profile:"
    flag                    = fscanf(ftxt, "%s", data.order_profile); // order_profile
    data.size_order_profile = strlen((const char *)data.order_profile);
  }
  fclose(ftxt);

  //   char *c = UTF82Char((const char*)data.order_profile);

  uint8_t *data_encrypt = new uint8_t[1024];
  int nData;
  encrypt_data_from_backend_to_pc(data_encrypt, nData, data);

  FILE *fbin = fopen(binaryName, "wb");
  fwrite(data_encrypt, 1, nData, fbin);
  fclose(fbin);

  delete[] data_encrypt;
  xcb_auth_backend_data_free(data);

  return 0;
}

#endif
