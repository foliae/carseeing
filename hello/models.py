from django.db import models
from django.utils import timezone

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