/*
    SPDX-FileCopyrightText: 2006-2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#define USES_DEPRECATED_MAILTRANSPORT_API

#include <QWidget>
#include <widgets/transportcombobox.h>

class KJob;
class QLineEdit;
class QPlainTextEdit;

class TransportMgr : public QWidget
{
    Q_OBJECT

public:
    TransportMgr();

private Q_SLOTS:
    void removeAllBtnClicked();
    void editBtnClicked();
    void sendBtnClicked();
    void cancelBtnClicked();
    void jobResult(KJob *job);
    void jobPercent(KJob *job, unsigned long percent);
    void jobInfoMessage(KJob *job, const QString &info);

private:
    MailTransport::TransportComboBox *mComboBox = nullptr;
    QLineEdit *mSenderEdit = nullptr;
    QLineEdit *mToEdit = nullptr;
    QLineEdit *mCcEdit = nullptr;
    QLineEdit *mBccEdit = nullptr;
    QPlainTextEdit *mMailEdit = nullptr;
    KJob *mCurrentJob = nullptr;
};
