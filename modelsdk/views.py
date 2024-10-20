from django.shortcuts import redirect, render
from django.utils.timezone import datetime
from django.views.generic import ListView
from django.contrib import messages
from django.contrib.admin.views.decorators import staff_member_required
from django.http import JsonResponse
from .models import Order
from django.views.decorators.csrf import csrf_exempt

from .forms import CompanyForm
from .cparser import call_parser
from modelsdk.forms import LogMessageForm
from modelsdk.models import LogMessage
import logging



class HomeListView(ListView):
    """Renders the home page, with a list of all polls."""

    model = LogMessage

    def get_context_data(self, **kwargs):
        context = super(HomeListView, self).get_context_data(**kwargs)
        return context


def about(request):
    """Renders the about page."""
    return render(request, "modelsdk/about.html")


def contact(request):
    """Renders the contact page."""
    return render(request, "modelsdk/contact.html")


def modelsdk_there(request, name):
    """Renders the modelsdk_there page.
    Args:
        name: Name to say hello to
    """
    return render(
        request, "modelsdk/modelsdk_there.html", {"name": name, "date": datetime.now()}
    )


def log_message(request):
    form = LogMessageForm(request.POST or None)
    if request.method == "POST":
        if form.is_valid():
            message = form.save(commit=False)
            message.log_date = datetime.now()
            message.save()
            return redirect("home")
        else:
            return render(request, "modelsdk/log_message.html", {"form": form})
    else:
        return render(request, "modelsdk/log_message.html", {"form": form})

@staff_member_required
def add_data(request):
    if request.method == 'POST':
        form_type = request.POST.get('form_type')
        if form_type == 'company':
            form = CompanyForm(request.POST)
        #elif form_type == 'order':
            #form = OrderForm(request.POST)
        #elif form_type == 'pc':
            #form = PCForm(request.POST)
        
        if form.is_valid():
            form.save()
            messages.success(request, f'{form_type.capitalize()} 添加成功！')
            return redirect('add_data')
        else:
            messages.error(request, '添加失败，请检查输入。')
    else:
        company_form = CompanyForm()
        #order_form = OrderForm()
        #pc_form = PCForm()
    
    return render(request, 'modelsdk/add_data.html', {
        'company_form': company_form,
        #'order_form': order_form,
        #'pc_form': pc_form,
    })

@staff_member_required
def get_orders_for_company(request):
    company_id = request.GET.get('company_id')
    orders = Order.objects.filter(company_id=company_id).values('id', 'order_num')
    return JsonResponse({'orders': list(orders)})


logger = logging.getLogger(__name__)
#curl -X POST -m 5 -H "Content-Length: 96" --data-binary @test.bin http://127.0.0.1:8000/xcb/
@csrf_exempt
def xcb(request):
    if request.method == 'POST':
        # 调用 Cython 中的解析函数并获取返回值
        if len(request.body) > 0:
            result = call_parser.parse_data(request.body)
            result['body_len'] = len(request.body)
            return JsonResponse({'status': 'success', 'result': result}, status=200)
        return JsonResponse({'status': 'error', 'message': 'body len = 0'}, status=400)

    return JsonResponse({'status': 'error', 'message': 'Invalid request method.'}, status=400)