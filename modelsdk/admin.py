from django.contrib import admin
from . import models
from django.contrib.admin import SimpleListFilter
from django.db.models import Q
from django import forms
from django.utils import timezone
from datetime import timedelta

"""
class CustomAdminSite(admin.AdminSite):
    site_header = '企业后台'
    site_title = '企业后台'
    index_title = '企业后台'

    def get_model_perms(self, request):
        perms = super().get_model_perms(request)
        # 自定义模型顺序
        ordered_models = ['Company', 'Order', 'PC']  # 替换为您的模型名称
        return {model: perms[model] for model in ordered_models if model in perms}

    def get_urls(self):  # 添加此方法以自定义模型顺序
        urls = super().get_urls()
        # 根据 ordered_models 重新排序
        ordered_models = ['Company', 'Order', 'PC']
        model_urls = [url for url in urls if any(model.lower() in url.pattern.regex.pattern.lower() for model in ordered_models)]
        return model_urls + [url for url in urls if url not in model_urls]

    def has_permission(self, request):
        # 确保用户有权限访问
        return request.user.is_active and request.user.is_staff
admin_site = CustomAdminSite(name='custom_admin')
admin_site.register(Group)  # 注册组模型
admin_site.register(User)  # 注册用户模型
"""
class TotalUsedFilter(SimpleListFilter):
    title = 'Total Used'  # 显示在筛选框上的标题
    parameter_name = 'totalused'  # URL 参数名

    def lookups(self, request, model_admin):
        return (
            ('gt10', '大于 10'),
            ('gt50', '大于 50'),
            ('gt100', '大于 100'),
        )

    def queryset(self, request, queryset):
        if self.value() == 'gt10':
            return queryset.filter(totalused__gt=10)
        if self.value() == 'gt50':
            return queryset.filter(totalused__gt=50)
        if self.value() == 'gt100':
            return queryset.filter(totalused__gt=100)

@admin.register(models.Company)
class CompanyAdmin(admin.ModelAdmin):
    search_fields = ['company__icontains']  # 启用对 company 字段的搜索
    list_display = ['company', 'total_max', 'totalused', 'created_at']  # 在列表中显示这些字段
    list_filter = ('created_at',TotalUsedFilter,)  # 使用自定义的筛选器
    list_per_page = 20  # 每页显示的记录数

class MaxEmbedFilter(SimpleListFilter):
    title = 'Max Embed'
    parameter_name = 'max_embed'

    def lookups(self, request, model_admin):
        return (
            ('lt100', '小于 100'),
            ('100to500', '100 到 500'),
            ('gt500', '大于 500'),
        )

    def queryset(self, request, queryset):
        if self.value() == 'lt100':
            return queryset.filter(max_embed__lt=100)
        if self.value() == '100to500':
            return queryset.filter(max_embed__gte=100, max_embed__lte=500)
        if self.value() == 'gt500':
            return queryset.filter(max_embed__gt=500)

@admin.register(models.Order)
class OrderAdmin(admin.ModelAdmin):
    search_fields = ['order_num', 'order_profile', 'company__company']  # 启用对这些字段的搜索
    list_display = ['order_num', 'order_profile', 'company', 'max_embed', 'alreadyused','created_at']  # 在列表中显示这些字段
    list_filter = ('company', MaxEmbedFilter,'created_at')  # 使用公司和自定义的 MaxEmbedFilter 进行筛选
    list_per_page = 20  # 每页显示的记录数

    def get_search_results(self, request, queryset, search_term):
        queryset, use_distinct = super().get_search_results(request, queryset, search_term)
        try:
            search_term_as_int = int(search_term)
            queryset |= self.model.objects.filter(max_embed=search_term_as_int)
        except ValueError:
            pass
        return queryset, use_distinct


class MaxEmbedPCFilter(SimpleListFilter):
    title = 'Max Embed'
    parameter_name = 'max_embed'

    def lookups(self, request, model_admin):
        return (
            ('lt100', '小于 100'),
            ('100to500', '100 到 500'),
            ('gt500', '大于 500'),
        )

    def queryset(self, request, queryset):
        if self.value() == 'lt100':
            return queryset.filter(order__max_embed__lt=100)
        if self.value() == '100to500':
            return queryset.filter(order__max_embed__gte=100, order__max_embed__lte=500)
        if self.value() == 'gt500':
            return queryset.filter(order__max_embed__gt=500)


class PCAdminForm(forms.ModelForm):
    class Meta:
        model = models.PC
        fields = '__all__'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if 'company' in self.data:
            try:
                company_id = int(self.data.get('company'))
                self.fields['order'].queryset = models.Order.objects.filter(company_id=company_id)
            except (ValueError, TypeError):
                pass
        elif self.instance.pk and self.instance.company:
            self.fields['order'].queryset = models.Order.objects.filter(company=self.instance.company)

@admin.register(models.PC)
class PCAdmin(admin.ModelAdmin):
    form = PCAdminForm
    search_fields = ['pc_id', 'auth_code', 'order__order_num', 'order__company__company']
    list_display = ['pc_id', 'auth_code', 'get_order_num', 'get_company', 'get_max_embed', 'get_already_used', 'created_at']
    list_filter = ('order__company', MaxEmbedPCFilter, 'order__alreadyused', 'created_at')
    list_per_page = 20
    #readonly_fields = ['company']

    def get_order_num(self, obj):
        return obj.order.order_num
    get_order_num.short_description = '订单号'
    get_order_num.admin_order_field = 'order__order_num'

    def get_company(self, obj):
        return obj.order.company.company
    get_company.short_description = '公司'
    get_company.admin_order_field = 'order__company__company'

    def get_max_embed(self, obj):
        return obj.order.max_embed
    get_max_embed.short_description = '最大授权数'
    get_max_embed.admin_order_field = 'order__max_embed'

    def get_already_used(self, obj):
        return obj.order.alreadyused
    get_already_used.short_description = '已使用次数'
    get_already_used.admin_order_field = 'order__alreadyused'

    def get_search_results(self, request, queryset, search_term):
        queryset, use_distinct = super().get_search_results(request, queryset, search_term)
        try:
            search_term_as_int = int(search_term)
            queryset |= self.model.objects.filter(
                Q(order__max_embed=search_term_as_int) |
                Q(order__alreadyused=search_term_as_int)
            )
        except ValueError:
            pass
        return queryset, use_distinct

    def formfield_for_foreignkey(self, db_field, request, **kwargs):
        if db_field.name == "order":
            # 获取 URL 参数中的时间范围
            time_range = request.GET.get('time_range', 'all')
            
            # 根据时间范围过滤 Order
            if time_range == 'today':
                kwargs["queryset"] = models.Order.objects.filter(created_at__date=timezone.now().date())
            elif time_range == 'week':
                kwargs["queryset"] = models.Order.objects.filter(created_at__gte=timezone.now() - timedelta(days=7))
            elif time_range == 'month':
                kwargs["queryset"] = models.Order.objects.filter(created_at__gte=timezone.now() - timedelta(days=30))
            else:
                kwargs["queryset"] = models.Order.objects.all()

        return super().formfield_for_foreignkey(db_field, request, **kwargs)

    def add_view(self, request, form_url='', extra_context=None):
        extra_context = extra_context or {}
        extra_context['time_range_form'] = TimeRangeForm(request.GET)
        return super().add_view(request, form_url, extra_context=extra_context)

    #change_form_template = 'modelsdk/pc/add_form.html'

    def get_companies(self):
        return models.Company.objects.all()

    def get_queryset(self, request):
        qs = super().get_queryset(request)
        company_id = request.GET.get('company')
        if company_id:
            qs = qs.filter(company_id=company_id)
        return qs

    class Media:
        js = ('pc_admin.js',)

class TimeRangeForm(forms.Form):
    TIME_RANGES = [
        ('all', '所有时间'),
        ('today', '今天'),
        ('week', '最近一周'),
        ('month', '最近一个月'),
    ]
    time_range = forms.ChoiceField(choices=TIME_RANGES, label='订单创建时间', required=False)



# 保留其他已有的注册和设置
#admin.site.register(models.LogMessage)
admin.site.site_header = ''
admin.site.site_title = '企业后台'
admin.site.index_title = '企业后台'
#admin.site.register(models.NormalUser)

@admin.register(models.Embed)
class EmbedAdmin(admin.ModelAdmin):
    search_fields = ['embed_id', 'pc__pc_id', 'order__order_num', 'company__company']  # 启用对这些字段的搜索
    list_display = ['embed_id', 'get_pc_id', 'get_order_num', 'get_company', 'id_count', 'updated_at']  # 在列表中显示这些字段
    list_filter = ('company', 'order', 'updated_at')  # 使用公司和订单进行筛选
    list_per_page = 20  # 每页显示的记录数

    def get_pc_id(self, obj):
        return obj.pc.pc_id
    get_pc_id.short_description = 'PC ID'
    get_pc_id.admin_order_field = 'pc__pc_id'

    def get_order_num(self, obj):
        return obj.order.order_num
    get_order_num.short_description = '订单号'
    get_order_num.admin_order_field = 'order__order_num'

    def get_company(self, obj):
        return obj.company.company
    get_company.short_description = '公司'
    get_company.admin_order_field = 'company__company'

    def get_queryset(self, request):
        qs = super().get_queryset(request)
        company_id = request.GET.get('company')
        if company_id:
            qs = qs.filter(company_id=company_id)
        return qs
