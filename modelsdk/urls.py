from django.urls import path
from django.views.generic.base import RedirectView
from django.contrib import admin
from modelsdk import views
from modelsdk.models import LogMessage

home_list_view = views.HomeListView.as_view(
    queryset=LogMessage.objects.order_by("-log_date")[:5],  # :5 limits the results to the five most recent
    context_object_name="message_list",
    template_name="modelsdk/home.html",
)

urlpatterns = [
    path('admin/', admin.site.urls),
    path("", home_list_view, name="home"),
    path("modelsdk/<name>", views.modelsdk_there, name="modelsdk_there"),
    path("about/", views.about, name="about"),
    path("contact/", views.contact, name="contact"),
    path("log/", views.log_message, name="log"),
    path('get_orders_for_company/', views.get_orders_for_company, name='get_orders_for_company'),
    path('favicon.ico', RedirectView.as_view(url='/static/favicon.ico')), 
    path('xcb/', views.xcb, name='xcb'),  # 添加新的 URL 路径
]
