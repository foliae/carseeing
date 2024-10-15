from django import forms

from modelsdk.models import LogMessage
from .models import Company

class LogMessageForm(forms.ModelForm):
    class Meta:
        model = LogMessage
        fields = ("message",)  # NOTE: the trailing comma is required


class CompanyForm(forms.ModelForm):
    class Meta:
        model = Company
        fields = ['company']

    def clean_company(self):
        company_name = self.cleaned_data['company']
        if Company.objects.filter(company__iexact=company_name).exists():
            raise forms.ValidationError("该公司已存在。")
        return company_name