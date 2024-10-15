(function($) {
    'use strict';
    $(document).ready(function() {
        console.log("PC admin script loaded");
        var $companySelect = $('#id_company'); // 获取公司选择下拉框
        var $orderSelect = $('#id_order'); // 获取订单选择下拉框

        function updateOrders() {
            console.log("updateOrders called");
            var companyId = $companySelect.val(); // 获取当前选择的公司 ID
            console.log("Selected company ID:", companyId);
            if (companyId) {
                // 发送 AJAX 请求获取与所选公司相关的订单
                $.getJSON('/get_orders_for_company/', {company_id: companyId}, function(data) {
                    console.log("Received orders:", data);
                    $orderSelect.empty(); // 清空订单下拉框
                    $orderSelect.append($('<option></option>').attr('value', '').text('---------')); // 添加默认选项
                    $.each(data.orders, function(index, order) {
                        $orderSelect.append($('<option></option>').attr('value', order.id).text(order.order_num)); // 添加订单选项
                    });
                }).fail(function(jqXHR, textStatus, errorThrown) {
                    console.error("AJAX request failed:", textStatus, errorThrown); // 处理请求失败
                });
            } else {
                $orderSelect.empty(); // 如果没有选择公司，清空订单下拉框
                $orderSelect.append($('<option></option>').attr('value', '').text('---------')); // 添加默认选项
            }
        }

        $companySelect.on('change', updateOrders); // 绑定公司选择框的变化事件
        console.log("Change event handler attached to company select");

        // 页面加载时也更新一次
        updateOrders();
    });
})(jQuery); // 使用 jQuery 作为参数