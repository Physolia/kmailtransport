/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MAILTRANSPORT_ERRORATTRIBUTE_H
#define MAILTRANSPORT_ERRORATTRIBUTE_H

#include <mailtransportakonadi_export.h>

#include <QString>

#include <attribute.h>

namespace MailTransport {
/**
 * @short An Attribute to mark messages that failed to be sent.
 *
 * This attribute contains the error message encountered.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 * @since 4.4
 */
class MAILTRANSPORTAKONADI_EXPORT ErrorAttribute : public Akonadi::Attribute
{
public:
    /**
     * Creates a new error attribute.
     *
     * @param msg The i18n'ed error message.
     */
    explicit ErrorAttribute(const QString &msg = QString());

    /**
     * Destroys the error attribute.
     */
    ~ErrorAttribute() override;

    /**
     * Returns the i18n'ed error message.
     */
    QString message() const;

    /**
     * Sets the i18n'ed error message.
     */
    void setMessage(const QString &msg);

    /* reimpl */
    ErrorAttribute *clone() const override;
    QByteArray type() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}

#endif
