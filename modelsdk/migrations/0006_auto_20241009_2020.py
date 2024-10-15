# Generated by Django 3.x.x on YYYY-MM-DD HH:MM

from django.db import migrations, models
import django.db.models.deletion

class Migration(migrations.Migration):

    dependencies = [
        ('modelsdk', '0005_order_created_at'),
    ]

    operations = [
        migrations.AddField(
            model_name='pc',
            name='company',
            field=models.ForeignKey('Company', on_delete=django.db.models.deletion.CASCADE, related_name='pcs', null=True, blank=True),
        ),
    ]