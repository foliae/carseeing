cdef extern from "parser.h":
    ctypedef enum xcb_auth_event_code_t:
        # *******************Login/Logout******************
        # Request login 请求登录
        XCB_EVENT_LOGIN_REQUEST = 0x0001
        # Login allowed 允许登录
        XCB_EVENT_LOGIN_ALLOWED = 0x0002
        # Login rejected 登录被拒绝
        XCB_EVENT_LOGIN_REJECTED = 0x0003
        # Request logout 登录退出
        XCB_EVENT_LOGOUT_REQUEST = 0x0004
        # Undefined "login/logout" event! 未识别的登录或退出事件
        XCB_EVENT_LOGINOUT_UNDEFINED = 0x000f

        # *****************Authorization*******************
        # Request authorization 请求授权
        XCB_EVENT_AUTH_REQUEST = 0x1001
        # Authorization allowed 允许授权
        XCB_EVENT_AUTH_ALLOWED = 0x1002
        # Authorization rejected 授权被拒绝
        XCB_EVENT_AUTH_REJECTED = 0x1003
        # Undefined "authorization" event! 未识别的授权事件
        XCB_EVENT_AUTH_UNDEFINED = 0x100f

        # ********create authentication file or not********
        # Success to create authentication file 鉴权文件生成成功
        XCB_EVENT_CREATE_FILE_SUCCESS = 0x2001
        # Fail to create authentication file 鉴权文件生成失败
        XCB_EVENT_CREATE_FILE_FAIL = 0x2002
        # Undefined "create authentication file" event! 未识别的鉴权文件生成事件
        XCB_EVENT_CREATE_FILE_UNDEFINED = 0x200f

        # *****************query order************************
        # Query order 查询订单
        XCB_EVENT_ORDER_QUERY = 0x3001
        # Valid order status 订单状态有效
        XCB_EVENT_ORDER_VALID = 0x3002
        # Invalid order status 订单状态无效
        XCB_EVENT_ORDER_INVALID = 0x3003

        # Undefined event code! 未识别的事件码
        XCB_EVENT_CODE_UNDEFINED = 0xf000

    ctypedef struct xcb_auth_backend_data_t:
        unsigned char size_pc_id          # Bytes of PC ID
        unsigned char size_auth_code      # Bytes of Authorization code
        unsigned char size_embed_id       # Bytes of embedded ID
        unsigned char size_check_code     # Bytes of check code
        unsigned char size_company        # Bytes of company name
        unsigned char size_project        # Bytes of project name
        unsigned short size_order_profile # Bytes of order profile
        unsigned char type_embed_id       # Type of embed id, 0x07-CPU ID
        xcb_auth_event_code_t event      # Event code
        unsigned char *pc_id              # PC ID
        unsigned char *auth_code          # Authorization code(bound with PC ID)
        unsigned char *embedded_id        # Embedded ID
        unsigned char *check_code         # Check code
        unsigned char *company            # Company name
        unsigned char *project            # Project name("order number" actually)
        unsigned char *order_profile      # Order profiles
        unsigned int times_remaining       # Remaining authorization times
        unsigned int times_total           # Total authorization times

    xcb_auth_backend_data_t parse_binary_data(const unsigned char *data, size_t length)

    void xcb_auth_backend_data_free(xcb_auth_backend_data_t &data)

def parse_data(bytes binary_data):
    cdef size_t length = len(binary_data)
    cdef const unsigned char *c_data = binary_data
    cdef xcb_auth_backend_data_t parsed_data = parse_binary_data(c_data, length)

    # 创建 Python 字典
    result = {
        'size_pc_id': parsed_data.size_pc_id,
        'size_auth_code': parsed_data.size_auth_code,
        'size_embed_id': parsed_data.size_embed_id,
        'size_check_code': parsed_data.size_check_code,
        'size_company': parsed_data.size_company,
        'size_project': parsed_data.size_project,
        'size_order_profile': parsed_data.size_order_profile,
        'type_embed_id': parsed_data.type_embed_id,
        'event_code': parsed_data.event,
        'pc_id': parsed_data.pc_id[:parsed_data.size_pc_id].decode('utf-8'),  # 将 C 字符串转换为 Python 字符串
        'auth_code': parsed_data.auth_code[:parsed_data.size_auth_code].decode('utf-8'),
        'embed_id': parsed_data.embedded_id[:parsed_data.size_embed_id].decode('utf-8'),
        'check_code': parsed_data.check_code[:parsed_data.size_check_code].decode('utf-8'),
        'company': parsed_data.company[:parsed_data.size_company].decode('utf-8'),
        'order_num': parsed_data.project[:parsed_data.size_project].decode('utf-8'),
        'order_profile': parsed_data.order_profile[:parsed_data.size_order_profile].decode('utf-8'),
        'times_remaining': parsed_data.times_remaining,
        'times_total': parsed_data.times_total
    }
    xcb_auth_backend_data_free(parsed_data)
    return result
