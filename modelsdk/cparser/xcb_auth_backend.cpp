#include "xcb_auth_backend.h"

/** Initialize data struct of authorization information
 *
 * @param[in] data: data of authorization information
 *
 * @return null
 */
void xcb_auth_backend_data_init(xcb_auth_backend_data_t &data)
{
  data.pc_id              = new unsigned char[SIZE_MAX_PC_ID];
  data.auth_code          = new unsigned char[SIZE_MAX_AUTH_CODE];
  data.embedded_id        = new unsigned char[SIZE_MAX_EMBED_ID];
  data.check_code         = new unsigned char[SIZE_MAX_CHECK_CODE];
  data.company            = new unsigned char[SIZE_MAX_COMPANY];
  data.project            = new unsigned char[SIZE_MAX_PROJECT];
  data.order_profile      = new unsigned char[SIZE_MAX_ORDER_PROFILE];
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



static uint8_t key_pc[32] = {0x4a, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0xc5, 0xae,
                             0xf0, 0x85, 0x7d, 0xaa, 0x81, 0x1f, 0xf5, 0x2c, 0x17, 0x3b, 0x9e,
                             0x08, 0xd3, 0x2d, 0x9a, 0x30, 0xa7, 0x33, 0x14, 0xdf, 0xf4};

static uint8_t key_backend[32] = {0xca, 0x5d, 0xeb, 0x14, 0x15, 0xca, 0x70, 0xbe, 0x2b, 0xc5, 0xae,
                                  0xf0, 0x87, 0x7d, 0x4a, 0x82, 0x17, 0x77, 0x2c, 0x57, 0x7b, 0x9e,
                                  0x08, 0xd5, 0x8d, 0x3e, 0x50, 0xa7, 0x53, 0x1c, 0xd5, 0xfc};

static uint8_t key_embed[32] = {0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x2b, 0xc5,
                                0xae, 0xf0, 0x87, 0x7d, 0x4a, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
                                0xad, 0x2b, 0x41, 0xd5, 0x8d, 0x3e, 0x50, 0xa7, 0x53, 0x1c};

/** Decrypt data from pc
 *
 * @param[in] data_cipher: Cipher text got from pc
 * @param[in] nData: Length of "data_cipher"
 * @param[out] data: Authorization information
 *
 * @return true - decrypt authorization information successfully,
 *         false - Fail to decrypt authorization information
 */
bool decrypt_data_from_pc_to_backend(const unsigned char *data_cipher,
                                     int nData,
                                     xcb_auth_backend_data_t &data)
{
  if (!data_cipher || nData < AES_BLOCKLEN)
    return false;

  unsigned char tmp_data[AES_BLOCKLEN];
  aes_cbc_decrypt(tmp_data, data_cipher, AES_BLOCKLEN, key_pc, 32, NULL);

  int ipos_size_data       = 1;
  unsigned short size_data = *((unsigned short *)(tmp_data + ipos_size_data));
  if (size_data > (unsigned short)nData || size_data % AES_BLOCKLEN != 0)
    return false;

  unsigned char *data1 = new unsigned char[size_data];
  aes_cbc_decrypt(data1, data_cipher, size_data, key_pc, 32, NULL);

  int ipos_event_code      = 4;
  int ipos_pc_id_bytes     = 7;
  int ipos_pc_id           = 8;
  int ipos_auth_code_bytes = 59;
  int ipos_auth_code       = 60;

  unsigned short event_code = *((unsigned short *)(data1 + ipos_event_code));
  unsigned short event_main = event_code & 0xf000;
  if (event_main == 0x0000) // login/logout
  {
    if (event_code == 0x0001)
    {
      data.event = XCB_EVENT_LOGIN_REQUEST; // login
    }
    else if (event_code == 0x0004)
    {
      data.event = XCB_EVENT_LOGOUT_REQUEST; // logout
    }
    else
    {
      data.event = XCB_EVENT_LOGINOUT_UNDEFINED; // undefined
    }

    data.size_pc_id = data1[ipos_pc_id_bytes];
    memcpy(data.pc_id, data1 + ipos_pc_id, data.size_pc_id);

    data.size_auth_code = data1[ipos_auth_code_bytes];
    memcpy(data.auth_code, data1 + ipos_auth_code, data.size_auth_code);

    data.size_embed_id      = 0;
    data.size_check_code    = 0;
    data.size_company       = 0;
    data.size_project       = 0;
    data.size_order_profile = 0;
    data.times_remaining    = 0;
  }
  else if (event_main == 0x1000) // Authorization
  {
    if (event_code == 0x1001)
    {
      data.event = XCB_EVENT_AUTH_REQUEST; // Request authorization
    }
    else
    {
      data.event = XCB_EVENT_AUTH_UNDEFINED; // undefined
    }
    int ipos_order_num_bytes  = 71;
    int ipos_order_num        = 72;
    int ipos_embed_data_bytes = 93;
    int ipos_embed_data       = 96;

    data.size_pc_id = data1[ipos_pc_id_bytes];
    memcpy(data.pc_id, data1 + ipos_pc_id, data.size_pc_id);

    data.size_auth_code = data1[ipos_auth_code_bytes];
    memcpy(data.auth_code, data1 + ipos_auth_code, data.size_auth_code);

    data.size_project = data1[ipos_order_num_bytes];
    memcpy(data.project, data1 + ipos_order_num, data.size_project);

    unsigned short bytes_embed_data = *((unsigned short *)(data1 + ipos_embed_data_bytes));
    if (bytes_embed_data % AES_BLOCKLEN != 0 || bytes_embed_data > size_data - ipos_embed_data)
    {
      data.size_embed_id   = 0;
      data.size_check_code = 0;
      data.size_company    = 0;
      data.times_remaining = 0;

      delete[] data1;
      return false;
    }

    unsigned char *data_embed = new unsigned char[bytes_embed_data];
    aes_cbc_decrypt(data_embed, data1 + ipos_embed_data, bytes_embed_data, key_embed, 32, NULL);

    int ipos_type_embed_id  = 1;
    int ipos_check_code     = 3;
    int ipos_embed_id_bytes = 8;
    int ipos_embed_id       = 9;

    data.type_embed_id = data_embed[ipos_type_embed_id];

    data.size_check_code = 4;
    memcpy(data.check_code, data_embed + ipos_check_code, data.size_check_code);

    data.size_embed_id = data_embed[ipos_embed_id_bytes];
    memcpy(data.embedded_id, data_embed + ipos_embed_id, data.size_embed_id);

    data.size_company    = 0;
    data.times_remaining = 0;

    delete[] data_embed;
  }
  else if (event_main == 0x2000) // create authentication file or not
  {
    if (event_code == 0x2001)
    {
      data.event = XCB_EVENT_CREATE_FILE_SUCCESS; // Success to create
    }
    else if (event_code == 0x2002)
    {
      data.event = XCB_EVENT_CREATE_FILE_FAIL; // Fail to create
    }
    else
    {
      data.event = XCB_EVENT_CREATE_FILE_UNDEFINED; // undefined
    }
    int ipos_order_num_bytes  = 71;
    int ipos_order_num        = 72;
    int ipos_embed_data_bytes = 93;
    int ipos_embed_data       = 96;

    data.size_pc_id = data1[ipos_pc_id_bytes];
    memcpy(data.pc_id, data1 + ipos_pc_id, data.size_pc_id);

    data.size_auth_code = data1[ipos_auth_code_bytes];
    memcpy(data.auth_code, data1 + ipos_auth_code, data.size_auth_code);

    data.size_project = data1[ipos_order_num_bytes];
    memcpy(data.project, data1 + ipos_order_num, data.size_project);

    unsigned short bytes_embed_data = *((unsigned short *)(data1 + ipos_embed_data_bytes));
    if (bytes_embed_data % AES_BLOCKLEN != 0 || bytes_embed_data > size_data - ipos_embed_data)
    {
      data.size_embed_id   = 0;
      data.size_check_code = 0;
      data.size_company    = 0;
      data.times_remaining = 0;

      delete[] data1;
      return false;
    }

    unsigned char *data_embed = new unsigned char[bytes_embed_data];
    aes_cbc_decrypt(data_embed, data1 + ipos_embed_data, bytes_embed_data, key_embed, 32, NULL);

    int ipos_type_embed_id  = 1;
    int ipos_check_code     = 3;
    int ipos_embed_id_bytes = 8;
    int ipos_embed_id       = 9;

    data.type_embed_id = data_embed[ipos_type_embed_id];

    data.size_check_code = 4;
    memcpy(data.check_code, data_embed + ipos_check_code, data.size_check_code);

    data.size_embed_id = data_embed[ipos_embed_id_bytes];
    memcpy(data.embedded_id, data_embed + ipos_embed_id, data.size_embed_id);

    data.size_company    = 0;
    data.times_remaining = 0;

    delete[] data_embed;
  }
  else if (event_main == 0x3000)
  {
    if (event_code == 0x3001)
    {
      data.event               = XCB_EVENT_ORDER_QUERY;
      int ipos_order_num_bytes = 71;
      int ipos_order_num       = 72;

      data.size_pc_id = data1[ipos_pc_id_bytes];
      memcpy(data.pc_id, data1 + ipos_pc_id, data.size_pc_id);

      data.size_auth_code = data1[ipos_auth_code_bytes];
      memcpy(data.auth_code, data1 + ipos_auth_code, data.size_auth_code);

      data.size_project = data1[ipos_order_num_bytes];
      memcpy(data.project, data1 + ipos_order_num, data.size_project);
    }
    else
    {
      data.event = XCB_EVENT_CODE_UNDEFINED;
    }
  }
  else
  {
    data.event = XCB_EVENT_CODE_UNDEFINED; // undefined
  }

  delete[] data1;
  return true;
}

/** Encrypt data to pc
 *
 * @param[out] data_encrypt: Encrypted text(TODO: data space is created by
 * outside, maximum size is 1024bytes)
 * @param[out] nData: Real length of "data_encrypt"
 * @param[in] data: Authorization information
 *
 * @return true - encrypt authorization information successfully,
 *         false - Fail to encrypt authorization information
 */
bool encrypt_data_from_backend_to_pc(unsigned char *data_encrypt,
                                     int &nData,
                                     const xcb_auth_backend_data_t &data)
{
  if (data_encrypt == NULL)
    return false;

  nData = 0;

  std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
      std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
  auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  unsigned long long timestamp = tmp.count();

  XCB_RNG rng(timestamp);

  unsigned short event_code = data.event & 0xffff;
  unsigned short event_main = data.event & 0xf000;

  if (event_main == 0x0000)
  { // Login response
    unsigned short size_data = (70 / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
    unsigned char *data_raw  = new unsigned char[size_data];
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

    memcpy(data_raw + ipos_size_data, &size_data, sizeof(unsigned short));
    memcpy(data_raw + ipos_event_code, &event_code, sizeof(unsigned short));
    *(data_raw + ipos_pc_id_bytes) = data.size_pc_id;
    memcpy(data_raw + ipos_pc_id, data.pc_id, data.size_pc_id);
    *(data_raw + ipos_auth_code_bytes) = data.size_auth_code;
    memcpy(data_raw + ipos_auth_code, data.auth_code, data.size_auth_code);

    aes_cbc_encrypt(data_encrypt, data_raw, size_data, key_pc, 32, NULL);
    nData = size_data;

    delete[] data_raw;
  }
  else if (event_main == 0x1000)
  { // Authorization response
    // embed data encrypt
    unsigned short size_embed_data = 9 + data.size_embed_id;
    size_embed_data                = (size_embed_data / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
    unsigned char *data_embed_raw  = new unsigned char[size_embed_data];
    unsigned char *data_embed_enc  = new unsigned char[size_embed_data];
    for (unsigned short i = 0; i < size_embed_data; i++)
    {
      data_embed_raw[i] = (unsigned char)(rng.uniform(0, 256));
    }

    int ipos_embed_id_type  = 1;
    int ipos_check_code     = 3;
    int ipos_embed_id_bytes = 8;
    int ipos_embed_id       = 9;

    data_embed_raw[ipos_embed_id_type] = data.type_embed_id; // Embed ID type: 0x07-CPU ID
    memcpy(data_embed_raw + ipos_check_code, data.check_code, 4);
    data_embed_raw[ipos_embed_id_bytes] = data.size_embed_id;
    memcpy(data_embed_raw + ipos_embed_id, data.embedded_id, data.size_embed_id);
    aes_cbc_encrypt(data_embed_enc, data_embed_raw, size_embed_data, key_backend, 32, NULL);

    //  all data encrypt
    unsigned short size_data = 123 + size_embed_data;
    size_data                = (size_data / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
    unsigned char *data_raw  = new unsigned char[size_data];
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
    int ipos_remaining_times = 71;
    int ipos_company_bytes   = 76;
    int ipos_company_name    = 77;
    int ipos_project_bytes   = 98;
    int ipos_project_name    = 99;
    int ipos_embed_data_size = 120;
    int ipos_embed_data      = 123;

    memcpy(data_raw + ipos_size_data, &size_data, sizeof(unsigned short));
    memcpy(data_raw + ipos_event_code, &event_code, sizeof(unsigned short));
    *(data_raw + ipos_pc_id_bytes) = data.size_pc_id;
    memcpy(data_raw + ipos_pc_id, data.pc_id, data.size_pc_id);
    *(data_raw + ipos_auth_code_bytes) = data.size_auth_code;
    memcpy(data_raw + ipos_auth_code, data.auth_code, data.size_auth_code);
    memcpy(data_raw + ipos_remaining_times, &(data.times_remaining), sizeof(int));
    *(data_raw + ipos_company_bytes) = data.size_company;
    memcpy(data_raw + ipos_company_name, data.company, data.size_company);
    *(data_raw + ipos_project_bytes) = data.size_project;
    memcpy(data_raw + ipos_project_name, data.project, data.size_project);

    memcpy(data_raw + ipos_embed_data_size, &size_embed_data, sizeof(unsigned short));
    memcpy(data_raw + ipos_embed_data, data_embed_enc, size_embed_data);

    aes_cbc_encrypt(data_encrypt, data_raw, size_data, key_pc, 32, NULL);
    nData = size_data;

    delete[] data_raw;
    delete[] data_embed_raw;
    delete[] data_embed_enc;
  }
  else if (event_main == 0x3000) // order query
  {
    if (event_code == 0x3002) // order valid
    {
      unsigned short size_data = (605 / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
      unsigned char *data_raw  = new unsigned char[size_data];
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
      int ipos_order_num_bytes = 71;
      int ipos_order_num       = 72;

      int ipos_auth_num_total     = 93;
      int ipos_auth_num_remaining = 98;
      int ipos_oder_prof_bytes    = 103;
      int ipos_oder_prof          = 105;

      memcpy(data_raw + ipos_size_data, &size_data, sizeof(unsigned short));
      memcpy(data_raw + ipos_event_code, &event_code, sizeof(unsigned short));
      *(data_raw + ipos_pc_id_bytes) = data.size_pc_id;
      memcpy(data_raw + ipos_pc_id, data.pc_id, data.size_pc_id);
      *(data_raw + ipos_auth_code_bytes) = data.size_auth_code;
      memcpy(data_raw + ipos_auth_code, data.auth_code, data.size_auth_code);
      *(data_raw + ipos_order_num_bytes) = data.size_project;
      memcpy(data_raw + ipos_order_num, data.project, data.size_project);

      memcpy(data_raw + ipos_auth_num_total, &(data.times_total), sizeof(unsigned int));
      memcpy(data_raw + ipos_auth_num_remaining, &(data.times_remaining), sizeof(unsigned int));
      memcpy(data_raw + ipos_oder_prof_bytes, &(data.size_order_profile), sizeof(unsigned short));
      memcpy(data_raw + ipos_oder_prof, data.order_profile, data.size_order_profile);

      aes_cbc_encrypt(data_encrypt, data_raw, size_data, key_pc, 32, NULL);
      nData = size_data;

      delete[] data_raw;
    }
    else
    { // Invalid order
      event_code               = 0x3003;
      unsigned short size_data = (92 / AES_BLOCKLEN + 1) * AES_BLOCKLEN;
      unsigned char *data_raw  = new unsigned char[size_data];
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
      int ipos_order_num_bytes = 71;
      int ipos_order_num       = 72;

      memcpy(data_raw + ipos_size_data, &size_data, sizeof(unsigned short));
      memcpy(data_raw + ipos_event_code, &event_code, sizeof(unsigned short));
      *(data_raw + ipos_pc_id_bytes) = data.size_pc_id;
      memcpy(data_raw + ipos_pc_id, data.pc_id, data.size_pc_id);
      *(data_raw + ipos_auth_code_bytes) = data.size_auth_code;
      memcpy(data_raw + ipos_auth_code, data.auth_code, data.size_auth_code);
      *(data_raw + ipos_order_num_bytes) = data.size_project;
      memcpy(data_raw + ipos_order_num, data.project, data.size_project);

      aes_cbc_encrypt(data_encrypt, data_raw, size_data, key_pc, 32, NULL);
      nData = size_data;

      delete[] data_raw;
    }
  }

  return true;
}

// example: "113c25c63289" --> {0x11,0x3C,0x25,0xC6,0x32,0x89}
int xcb_str2hex(const uint8_t *str, uint8_t *hex, uint32_t len_str, uint32_t len_hex)
{
  uint32_t i;
  char buff[10], *endPtr;

  if (!str || !hex || len_str > 2 * len_hex)
    return -1;

  for (i = 0; i < len_str; i += 2)
  {
    sprintf(buff, "0x%c%c", str[i], str[i + 1]);
    *(hex++) = (uint8_t)strtol(buff, &endPtr, 16);
  }

  return 0;
}

// example: {0x11,0x3C,0x25,0xC6,0x32,0x89} --> "113c25c63289"
int xcb_hex2str(const uint8_t *hex, uint8_t *str, uint32_t len_hex, uint32_t len_str)
{
  uint32_t i;
  uint8_t h;
  uint8_t hex_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  if (!str || !hex || len_str < 2 * len_hex + 1)
    return -1;

  for (i = 0; i < len_hex; ++i)
  {
    h        = hex[i];
    *(str++) = hex_table[(h >> 4)];
    *(str++) = hex_table[(h & 0x0F)];
  }
  *str = '\0';

  return 0;
}
