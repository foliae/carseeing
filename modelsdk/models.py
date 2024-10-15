import uuid
from django.db import models
from django.utils import timezone
from django.core.exceptions import ValidationError

class LogMessage(models.Model):
    message = models.CharField(max_length=300)
    log_date = models.DateTimeField("date logged")

    def __str__(self):
        """Returns a string representation of a message."""
        date = timezone.localtime(self.log_date)
        return f"'{self.message}' logged on {date.strftime('%A, %d %B, %Y at %X')}"

class NormalUser(models.Model):
    username = models.CharField(max_length=32)
    password = models.CharField(max_length=32)
    user_type = models.IntegerField(choices=((1, '注册用户'), (2, '普通管理员'), (3, '超级管理员')), default=1)

    class Meta:
        # 定义这个配置
        verbose_name_plural = '用户表'

    def __str__(self):
        return self.username

class Company(models.Model):
    company = models.CharField(max_length=100, unique=True)
    totalused = models.PositiveIntegerField(default=0)
    created_at = models.DateTimeField(default=timezone.now, verbose_name="创建时间")
    search_fields = ('company', 'totalused', 'created_at')

    class Meta:
        verbose_name_plural = '公司'
        ordering = ['-created_at']  # 按照创建时间降序排列
        db_table = '公司列表'

    def __str__(self):
        return f"{self.company} (总使用次数: {self.totalused})"

    def update_totalused(self):
        self.totalused = sum(order.alreadyused for order in self.orders.all())
        self.save()

    def total_pcs(self):
        return sum(order.pcs.count() for order in self.orders.all())

class Order(models.Model):
    order_num = models.CharField(max_length=100, unique=True)
    order_profile = models.CharField(max_length=100, unique=True)
    company = models.ForeignKey(Company, on_delete=models.CASCADE, related_name='orders')
    max_embed = models.PositiveIntegerField(default=0)
    alreadyused = models.PositiveIntegerField(default=0)
    created_at = models.DateTimeField(default=timezone.now, verbose_name="创建时间")

    class Meta:
        verbose_name_plural = '订单'
        ordering = ['-created_at']  # 按照创建时间降序排列
        db_table = '订单'

    def __str__(self):
        return f"{self.order_num} (已使用: {self.alreadyused}/{self.max_embed})"

    def current_pc_count(self):
        return self.pcs.count()

    def update_alreadyused(self):
        self.alreadyused = sum(pc.total_embed_count() for pc in self.pcs.all())
        self.save()

    def save(self, *args, **kwargs):
        super().save(*args, **kwargs)
        self.company.update_totalused()  # 更新Company的totalused字段

class PC(models.Model):
    pc_id = models.CharField(max_length=100, unique=True)
    auth_code = models.CharField(max_length=32, unique=True, verbose_name="授权码", editable=False)
    company = models.ForeignKey('Company', on_delete=models.CASCADE, related_name='pcs', null=True, blank=True)
    order = models.ForeignKey(Order, on_delete=models.CASCADE, related_name='pcs')
    created_at = models.DateTimeField(default=timezone.now, verbose_name="创建时间")


    class Meta:
        verbose_name_plural = 'PC'
        ordering = ['-created_at']  # 按照创建时间降序排列
        db_table = 'PC'

    def __str__(self):
        return f"{self.pc_id} (授权码: {self.auth_code})"

    def clean(self):
        if self.order.current_pc_count() >= self.order.max_embed:
            raise ValidationError("PC数量已达到订单限制")

    def save(self, *args, **kwargs):
        if not self.auth_code:
            self.auth_code = self.generate_auth_code()
        self.clean()
        super().save(*args, **kwargs)

    @staticmethod
    def generate_auth_code():
        return uuid.uuid4().hex

    def total_embed_count(self):
        return self.embeds.count()

class Embed(models.Model):
    pc = models.ForeignKey(PC, on_delete=models.CASCADE, related_name='embeds')
    embed_id = models.CharField(max_length=100, unique=True)
    id_count = models.PositiveIntegerField(default=0)

    class Meta:
        verbose_name = 'Embed'
        db_table = 'Embed'

    def __str__(self):
        return f"Embed {self.embed_id} for {self.pc.pc_id} (Used: {self.id_count})"

    def clean(self):
        # 检查当前Order的所有Embed的id_count总和是否超过了Order的max_embed
        total_used = self.pc.order.alreadyused
        if total_used + self.id_count > self.pc.order.max_embed:
            raise ValidationError("Embed使用次数已超过订单限制")

    def save(self, *args, **kwargs):
        self.clean()
        super().save(*args, **kwargs)
        self.pc.order.update_alreadyused()  # 更新Order的alreadyused字段
