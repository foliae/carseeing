# Generated by Django 5.1.1 on 2024-09-29 10:58

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ("modelsdk", "0001_initial"),
    ]

    operations = [
        migrations.CreateModel(
            name="NormalUser",
            fields=[
                (
                    "id",
                    models.AutoField(
                        auto_created=True,
                        primary_key=True,
                        serialize=False,
                        verbose_name="ID",
                    ),
                ),
                ("username", models.CharField(max_length=32)),
                ("password", models.CharField(max_length=32)),
                (
                    "user_type",
                    models.IntegerField(
                        choices=[(1, "注册用户"), (2, "普通管理员"), (3, "超级管理员")],
                        default=1,
                    ),
                ),
            ],
            options={
                "verbose_name_plural": "用户表",
            },
        ),
    ]
