#ifndef __SERIALIZER_H
#define __SERIALIZER_H

#include <stdint.h>
#include "tarray.h"
#include "r_defs.h"

struct FWriter;
struct FReader;

extern TArray<sector_t>	loadsectors;
extern TArray<line_t>	loadlines;
extern TArray<side_t>	loadsides;
extern char nulspace[];



class FSerializer
{

public:
	FWriter *w = nullptr;
	FReader *r = nullptr;

	int ArraySize();

public:

	bool OpenWriter();
	bool OpenReader(const char *buffer, size_t length);
	void Close();
	void WriteKey(const char *key);
	bool BeginObject(const char *name);
	void EndObject();
	bool BeginArray(const char *name);
	void EndArray();
	void WriteObjects();
	const char *GetOutput(unsigned *len = nullptr);
	FSerializer &Args(const char *key, int *args, int *defargs, int special);
	FSerializer &Terrain(const char *key, int &terrain, int *def = nullptr);
	FSerializer &Sprite(const char *key, uint16_t &spritenum, uint16_t *def);
	FSerializer &StringPtr(const char *key, const char *&charptr);	// This only retrieves the address but creates no permanent copy of the string.
	bool isReading() const
	{
		return r != nullptr;
	}

	bool isWriting() const
	{
		return w != nullptr;
	}
	
	bool canSkip() const;

	template<class T>
	FSerializer &operator()(const char *key, T &obj)
	{
		return Serialize(*this, key, obj, (T*)nullptr);
	}

	template<class T>
	FSerializer &operator()(const char *key, T &obj, T &def)
	{
		return Serialize(*this, key, obj, &def);
	}

	template<class T>
	FSerializer &Array(const char *key, T *obj, int count, bool fullcompare = false)
	{
		if (fullcompare && isWriting() && !memcmp(obj, nulspace, count * sizeof(T)))
		{
			return *this;
		}

		if (BeginArray(key))
		{
			for (int i = 0; i < count; i++)
			{
				Serialize(*this, nullptr, obj[i], (T*)nullptr);
			}
			EndArray();
		}
		return *this;
	}

	template<class T>
	FSerializer &Array(const char *key, T *obj, T *def, int count, bool fullcompare = false)
	{
		if (fullcompare && isWriting() && def != nullptr && !memcmp(obj, def, count * sizeof(T)))
		{
			return *this;
		}
		if (BeginArray(key))
		{
			for (int i = 0; i < count; i++)
			{
				Serialize(*this, nullptr, obj[i], def ? &def[i] : nullptr);
			}
			EndArray();
		}
		return *this;
	}
};

FSerializer &Serialize(FSerializer &arc, const char *key, bool &value, bool *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, int64_t &value, int64_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, uint64_t &value, uint64_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, int32_t &value, int32_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, uint32_t &value, uint32_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, int8_t &value, int8_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, uint8_t &value, uint8_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, int16_t &value, int16_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, uint16_t &value, uint16_t *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, double &value, double *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, float &value, float *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, side_t *&value, side_t **defval);
FSerializer &Serialize(FSerializer &arc, const char *key, sector_t *&value, sector_t **defval);
FSerializer &Serialize(FSerializer &arc, const char *key, player_t *&value, player_t **defval);
FSerializer &Serialize(FSerializer &arc, const char *key, line_t *&value, line_t **defval);
FSerializer &Serialize(FSerializer &arc, const char *key, FTextureID &value, FTextureID *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, DObject *&value, DObject ** /*defval*/);
FSerializer &Serialize(FSerializer &arc, const char *key, FName &value, FName *defval);
FSerializer &Serialize(FSerializer &arc, const char *key, FDynamicColormap *&cm, FDynamicColormap **def);
FSerializer &Serialize(FSerializer &arc, const char *key, FSoundID &sid, FSoundID *def);
FSerializer &Serialize(FSerializer &arc, const char *key, PClassActor *&clst, PClassActor **def);
FSerializer &Serialize(FSerializer &arc, const char *key, FState *&state, FState **def);
FSerializer &Serialize(FSerializer &arc, const char *key, FStrifeDialogueNode *&node, FStrifeDialogueNode **def);
FSerializer &Serialize(FSerializer &arc, const char *key, FString *&pstr, FString **def);


template<class T>
FSerializer &Serialize(FSerializer &arc, const char *key, T *&value, T **)
{
	DObject *v = static_cast<DObject*>(value);
	Serialize(arc, key, v, nullptr);
	value = static_cast<T*>(v);
	return arc;
}

template<class T>
FSerializer &Serialize(FSerializer &arc, const char *key, TObjPtr<T> &value, TObjPtr<T> *)
{
	Serialize(arc, key, value.o, nullptr);
	return arc; 
}

template<class T, class TT>
FSerializer &Serialize(FSerializer &arc, const char *key, TArray<T, TT> &value, TArray<T, TT> *)
{
	if (arc.isWriting())
	{
		if (value.Size() == 0) return arc;	// do not save empty arrays
	}
	bool res = arc.BeginArray(key);
	if (arc.isReading())
	{
		if (!res)
		{
			value.Clear();
			return arc;
		}
		value.Resize(arc.ArraySize());
	}
	for (unsigned i = 0; i < value.Size(); i++)
	{
		Serialize(arc, nullptr, value[i], (T*)nullptr);
	}
	arc.EndArray();
	return arc;
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, DVector3 &p, DVector3 *def)
{
	return arc.Array<double>(key, &p[0], def? &(*def)[0] : nullptr, 3, true);
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, DRotator &p, DRotator *def)
{
	return arc.Array<DAngle>(key, &p[0], def? &(*def)[0] : nullptr, 3, true);
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, DVector2 &p, DVector2 *def)
{
	return arc.Array<double>(key, &p[0], def? &(*def)[0] : nullptr, 2, true);
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, DAngle &p, DAngle *def)
{
	return Serialize(arc, key, p.Degrees, def? &def->Degrees : nullptr);
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, PalEntry &pe, PalEntry *def)
{
	return Serialize(arc, key, pe.d, def? &def->d : nullptr);
}

inline FSerializer &Serialize(FSerializer &arc, const char *key, FRenderStyle &style, FRenderStyle *def)
{
	return Serialize(arc, key, style.AsDWORD, def ? &def->AsDWORD : nullptr);
}

template<class T, class TT>
FSerializer &Serialize(FSerializer &arc, const char *key, TFlags<T, TT> &flags, TFlags<T, TT> *def)
{
	return Serialize(arc, key, flags.Value, def? &def->Value : nullptr);
}


#endif