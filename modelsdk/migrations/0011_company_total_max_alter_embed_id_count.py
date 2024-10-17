# Generated by Django 5.1.2 on 2024-10-15 08:03

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('modelsdk', '0010_alter_embed_options_embed_company_embed_order_and_more'),
    ]

    operations = [
        migrations.AddField(
            model_name='company',
            name='total_max',
            field=models.PositiveIntegerField(default=0),
        ),
        migrations.AlterField(
            model_name='embed',
            name='id_count',
            field=models.PositiveIntegerField(default=1),
        ),
    ]
