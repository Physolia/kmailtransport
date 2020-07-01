/*
    SPDX-FileCopyrightText: 2017-2020 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAILTRANSPORT_SMTPCONFIGDIALOG_H
#define MAILTRANSPORT_SMTPCONFIGDIALOG_H

#include <QDialog>

namespace MailTransport {
class Transport;
class SMTPConfigWidget;
class SmtpConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SmtpConfigDialog(Transport *transport, QWidget *parent = nullptr);
    ~SmtpConfigDialog() override;

private:
    void okClicked();
    void slotTextChanged(const QString &text);

    Transport *mTransport = nullptr;
    SMTPConfigWidget *mConfigWidget = nullptr;
    QPushButton *mOkButton = nullptr;
};
} // namespace MailTransport

#endif
