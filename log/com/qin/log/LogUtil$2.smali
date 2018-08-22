.class final Lcom/qin/log/LogUtil$2;
.super Ljava/lang/Object;
.source "LogUtil.java"

# interfaces
.implements Lcom/qin/log/LogUtil$OnCallback;


# annotations
.annotation system Ldalvik/annotation/EnclosingMethod;
    value = Lcom/qin/log/LogUtil;->i(Ljava/lang/String;Ljava/lang/String;)V
.end annotation

.annotation system Ldalvik/annotation/InnerClass;
    accessFlags = 0x8
    name = null
.end annotation


# direct methods
.method constructor <init>()V
    .registers 1

    .prologue
    .line 24
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    return-void
.end method


# virtual methods
.method public onMessage(Ljava/lang/String;Ljava/lang/String;)V
    .registers 3

    .prologue
    .line 27
    invoke-static {p1, p2}, Landroid/util/Log;->i(Ljava/lang/String;Ljava/lang/String;)I

    .line 28
    return-void
.end method
