from django.contrib import admin

from . import models

admin.site.register(models.LogMessage)
admin.site.site_header = ''
admin.site.site_title = '企业后台'
admin.site.index_title = '企业后台'
admin.site.register(models.NormalUser)