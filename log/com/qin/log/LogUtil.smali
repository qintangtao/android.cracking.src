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
    const/16 v0, 0x7d0

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
    .registers 8

    .prologue
    const/4 v0, 0x0

    .line 42
    invoke-static {p1}, Landroid/text/TextUtils;->isEmpty(Ljava/lang/CharSequence;)Z

    move-result v1

    if-nez v1, :cond_83

    if-eqz p2, :cond_83

    .line 43
    const-string v1, "#%s#"

    const/4 v2, 0x1

    new-array v2, v2, [Ljava/lang/Object;

    invoke-static {}, Ljava/lang/System;->currentTimeMillis()J

    move-result-wide v3

    invoke-static {v3, v4}, Ljava/lang/String;->valueOf(J)Ljava/lang/String;

    move-result-object v3

    aput-object v3, v2, v0

    invoke-static {v1, v2}, Ljava/lang/String;->format(Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/String;

    move-result-object v1

    .line 45
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v2

    sget v3, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    if-le v2, v3, :cond_6f

    .line 46
    :goto_24
    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v2

    if-ge v0, v2, :cond_83

    .line 47
    sget v2, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v2, v0

    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v3

    if-ge v2, v3, :cond_52

    .line 48
    new-instance v2, Ljava/lang/StringBuilder;

    invoke-direct {v2}, Ljava/lang/StringBuilder;-><init>()V

    invoke-virtual {v2, v1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v2

    sget v3, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v3, v0

    invoke-virtual {p1, v0, v3}, Ljava/lang/String;->substring(II)Ljava/lang/String;

    move-result-object v3

    invoke-virtual {v2, v3}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v2

    invoke-virtual {v2}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object v2

    invoke-interface {p2, p0, v2}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    .line 46
    :goto_4e
    sget v2, Lcom/qin/log/LogUtil;->LOG_MAXLENGTH:I

    add-int/2addr v0, v2

    goto :goto_24

    .line 51
    :cond_52
    new-instance v2, Ljava/lang/StringBuilder;

    invoke-direct {v2}, Ljava/lang/StringBuilder;-><init>()V

    invoke-virtual {v2, v1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v2

    invoke-virtual {p1}, Ljava/lang/String;->length()I

    move-result v3

    invoke-virtual {p1, v0, v3}, Ljava/lang/String;->substring(II)Ljava/lang/String;

    move-result-object v3

    invoke-virtual {v2, v3}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v2

    invoke-virtual {v2}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object v2

    invoke-interface {p2, p0, v2}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    goto :goto_4e

    .line 56
    :cond_6f
    new-instance v0, Ljava/lang/StringBuilder;

    invoke-direct {v0}, Ljava/lang/StringBuilder;-><init>()V

    invoke-virtual {v0, v1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v0

    invoke-virtual {v0, p1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    move-result-object v0

    invoke-virtual {v0}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object v0

    invoke-interface {p2, p0, v0}, Lcom/qin/log/LogUtil$OnCallback;->onMessage(Ljava/lang/String;Ljava/lang/String;)V

    .line 59
    :cond_83
    return-void
.end method
