#ifndef PARAM_SFO_H
#define PARAM_SFO_H

#include <QString>

struct ParamSfoData {
    QString titleId;
    QString title;
    QString version;
    QString category;
    QString contentId;
};

ParamSfoData parseParamSfo(const QString& sfoPath);
bool saveParamSfo(const QString& sfoPath, const ParamSfoData& data);

#endif // PARAM_SFO_H
