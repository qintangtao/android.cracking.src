.class public Lcom/qin/log/LogUtil;
.super Ljava/lang/Object;
.source "LogUtil.java"


# annotations
.annotation system Ldalvik/annotation/MemberClasses;
    value = {
        Lcom/qin/log/LogUtil$OnCallback;
    }
.end annotation


# static fields
.field private static LOG_MAXLENGTH:I


# direct methods
.method static constructor <clinit>()V
    .registers 1

    .prologue
    .line 8
    const/16 v0, 0xfa0

    sput v0, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    return-void
.end method

.method public constructor <init>()V
    .registers 1

    .prologue
    .line 6
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    .line 10
    return-void
.end method

.method public static d(Ljava/lang/String;Ljava/lang/String;)V
    .registers 3

    .prologue
    .line 15
    new-instance v0, Lcom/qin/log/LogUtil$1;

    invoke-direct {v0}, Lcom/qin/log/LogUtil$1;-><init>()V

    invoke-static {p0, p1, v0}, Lcom/qin/log/LogUtil;->split(Ljava/lang/String;Ljava/lang/String;Lcom/qin/log/LogUtil$OnCallback;)V

    .line 21
    return-void
.end method

.method public static e(Ljava/lang/String;Ljava/lang/String;)V
    .registers 3

    .prologue
    .line 33
    new-instance v0, Lcom/qin/log/LogUtil$3;

    invoke-direct {v0}, Lcom/qin/log/LogUtil$3;-><init>()V

    invoke-static {p0, p1, v0}, Lcom/qin/log/LogUtil;->split(Ljava/lang/String;Ljava/lang/String;Lcom/qin/log/LogUtil$OnCallback;)V

    .line 39
    return-void
.end method

.method public static i(Ljava/lang/String;Ljava/lang/String;)V
    .registers 3

    .prologue
    .line 24
    new-instance v0, Lcom/qin/log/LogUtil$2;

    invoke-direct {v0}, Lcom/qin/log/LogUtil$2;-><init>()V

    invoke-static {p0, p1, v0}, Lcom/qin/log/LogUtil;->split(Ljava/lang/String;Ljava/lang/String;Lcom/qin/log/LogUtil$OnCallback;)V

    .line 30
    return-void
.end method

.method static split(Ljava/lang/String;Ljava/lang/String;Lcom/qin/log/LogUtil$OnCallback;)V
    .registers 6

    .prologue
    .line 42
    invoke-static {p1}, Landroid/text/TextUtils;->isEmpty(Ljava/lang/CharSequence;)Z

    move-result v0

    if-nez v0, :cond_13

    .line 43
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v0

    sget v1, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    if-le v0, v1, :cond_14

    .line 44
    if-eqz p2, :cond_13

    .line 45
    invoke-interface {p2, p0, p1}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    .line 63
    :cond_13
    return-void

    .line 48
    :cond_14
    const/4 v0, 0x0

    :goto_15
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v1

    if-ge v0, v1, :cond_13

    .line 49
    sget v1, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v1, v0

    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v2

    if-ge v1, v2, :cond_34

    .line 50
    if-eqz p2, :cond_30

    .line 51
    sget v1, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v1, v0

    invoke-virtual {p1, v0, v1}, Ljava/lang/String;->substring(II)Ljava/lang/String;

    move-result-object v1

    invoke-interface {p2, p0, v1}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    .line 48
    :cond_30
    :goto_30
    sget v1, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v0, v1

    goto :goto_15

    .line 55
    :cond_34
    if-eqz p2, :cond_30

    .line 56
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v1

    invoke-virtual {p1, v0, v1}, Ljava/lang/String;->substring(II)Ljava/lang/String;

    move-result-object v1

    invoke-interface {p2, p0, v1}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    goto :goto_30
.end method
