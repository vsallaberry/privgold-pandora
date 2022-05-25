#ifndef VS_UNIT_CSV_H
#define VS_UNIT_CSV_H

CSVRow LookupUnitRow(const std::string &name, const std::string &faction);
extern void AddMeshes(std::vector<Mesh*>&xmeshes, float&randomstartframe, float&randomstartseconds, float unitscale, const std::string &meshes,int faction,Flightgroup *fg,std::vector<unsigned int> *counts=NULL);

#endif // ! VS_UNIT_CSV_H
