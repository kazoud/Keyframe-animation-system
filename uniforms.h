#pragma once

#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

#include <glm/glm.hpp>
#include "glsupport.h"
#include "texture.h"

// Private namespace for some helper functions. You should ignore this unless you
// are interested in the internal implementation.
namespace _helper {
inline void genericGlUniformi(GLint location, int i) {
  ::glUniform1i(location, i);
}
inline void genericGlUniformf(GLint location, float f) {
  ::glUniform1f(location, f);
}
inline void genericGlUniformv(GLint location, int size, const GLint *v) {
  ::glUniform1iv(location, size, v);
}
inline void genericGlUniformv(GLint location, int size, const GLfloat *v) {
  ::glUniform1fv(location, size, v);
}
inline void genericGlUniformv(GLint location, int size, const glm::ivec1 *v) {
  ::glUniform1iv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::ivec2 *v) {
  ::glUniform2iv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::ivec3 *v) {
  ::glUniform3iv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::ivec4 *v) {
  ::glUniform4iv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::vec1 *v) {
  ::glUniform1fv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::vec2 *v) {
  ::glUniform2fv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::vec3 *v) {
  ::glUniform3fv(location, size, &v[0][0]);
}
inline void genericGlUniformv(GLint location, int size, const glm::vec4 *v) {
  ::glUniform4fv(location, size, &v[0][0]);                     // or glm::value_ptr(v[0])
}
inline void genericGlUniformMatrix4v(GLint location, int size, const glm::mat4 *m) {
  ::glUniformMatrix4fv(location, size, GL_FALSE, &m[0][0][0]);   // or glm::value_ptr(m[0])
}

template<typename T, int n>
inline GLenum getTypeForCvec();   // should replace with STATIC_ASSERT

template<>
inline GLenum getTypeForCvec<int, 1>() { return GL_INT; }
template<>
inline GLenum getTypeForCvec<int, 2>() { return GL_INT_VEC2; }
template<>
inline GLenum getTypeForCvec<int, 3>() { return GL_INT_VEC3; }
template<>
inline GLenum getTypeForCvec<int, 4>() { return GL_INT_VEC4; }
template<>
inline GLenum getTypeForCvec<float, 1>() { return GL_FLOAT; }
template<>
inline GLenum getTypeForCvec<float, 2>() { return GL_FLOAT_VEC2; }
template<>
inline GLenum getTypeForCvec<float, 3>() { return GL_FLOAT_VEC3; }
template<>
inline GLenum getTypeForCvec<float, 4>() { return GL_FLOAT_VEC4; }
template<>
inline GLenum getTypeForCvec<bool, 1>() { return GL_BOOL; }
template<>
inline GLenum getTypeForCvec<bool, 2>() { return GL_BOOL_VEC2; }
template<>
inline GLenum getTypeForCvec<bool, 3>() { return GL_BOOL_VEC3; }
template<>
inline GLenum getTypeForCvec<bool, 4>() { return GL_BOOL_VEC4; }
}

// The Uniforms keeps a map from strings to values
//
// Currently the value can be of the following type:
// - Single int, float, or glm::mat4
// - glm::vec<n, T> with T=int or float, and n = 1, 2, 3, or 4
// - shared_ptr<Texture>
// - arrays of any of the above
//
// You either use uniform.put("varName", val) or
// uniform.put("varArrayName", vals, numVals);
//
// A Uniforms instance will start off empty, and you can use
// its put member function to populate it.

class Uniforms {
public:
  Uniforms& put(const std::string& name, int value) {
    glm::ivec1 v(value);
    valueMap[name].reset(new CvecsValue<int, 1>(&v, 1));
    return *this;
  }

  Uniforms& put(const std::string& name, float value) {
    glm::vec1 v(value);
    valueMap[name].reset(new CvecsValue<float, 1>(&v, 1));
    return *this;
  }

  Uniforms& put(const std::string& name, const glm::mat4& value) {
    valueMap[name].reset(new Matrix4sValue(&value, 1));
    return *this;
  }

  Uniforms& put(const std::string& name, const std::shared_ptr<Texture>& value) {
    valueMap[name].reset(new TexturesValue(&value, 1));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, int>& v) {    // , glm::defaultp
    valueMap[name].reset(new CvecsValue<int, n>(&v, 1));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, float>& v) {
    valueMap[name].reset(new CvecsValue<float, n>(&v, 1));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, double>& v) {
    glm::vec<n, float> u;
    for (int i = 0; i < n; ++i) {
      u[i] = float(v[i]);
    }
    valueMap[name].reset(new CvecsValue<float, n>(&u, 1));
    return *this;
  }

  Uniforms& put(const std::string& name, const int *values, int count) {
    valueMap[name].reset(new CvecsValue<int, 1>(reinterpret_cast<const glm::ivec1*>(values), count));
    return *this;
  }

  Uniforms& put(const std::string& name, const float *values, int count) {
    valueMap[name].reset(new CvecsValue<float, 1>(reinterpret_cast<const glm::vec1*>(values), count));
    return *this;
  }

  Uniforms& put(const std::string& name, const glm::mat4 *values, int count) {
    valueMap[name].reset(new Matrix4sValue(values, count));
    return *this;
  }

  Uniforms& put(const std::string& name, const std::shared_ptr<Texture> *values, int count) {
    valueMap[name].reset(new TexturesValue(values, count));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, int> *v, int count) {
    valueMap[name].reset(new CvecsValue<int, n>(v, count));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, float> *v, int count) {
    valueMap[name].reset(new CvecsValue<float, n>(v, count));
    return *this;
  }

  template<int n>
  Uniforms& put(const std::string& name, const glm::vec<n, double> *v, int count) {
    valueMap[name].reset(new CvecsValue<float, n>(v, count));
    return *this;
  }

  // Future work: add put for different sized matrices, and array of basic types
protected:

  // Ghastly implementation details follow. Viewer be warned.

  friend class Material;
  class ValueHolder;
  class Value;

  typedef std::map<std::string, ValueHolder> ValueMap;

  ValueMap valueMap;

  const Value* get(const std::string& name) const {
    std::map<std::string, ValueHolder>::const_iterator i = valueMap.find(name);
    return i == valueMap.end() ? NULL : i->second.get();
  }

  class ValueHolder {
    Value *value_;

public:
    ValueHolder() : value_(NULL) {}
    ValueHolder(Value* value) : value_(value) {}
    ValueHolder(const ValueHolder& u) : value_(u.value_ ? u.value_->clone() : NULL) {}
    ~ValueHolder() {
      if (value_)
        delete value_;
    }
    void reset(Value* value) {
      if (value_)
        delete value_;
      value_ = value;
    }

    Value *get() const {
      return value_;
    }

    ValueHolder& operator= (const ValueHolder& u) {
      reset(u.value_ ? u.value_->clone() : NULL);
      return *this;
    }
  };


  class Value {
public:
    // One of the uniform type as returned by glGetActiveUniform, used for matching
    const GLenum type;

    // 1 for non-array type, otherwise the number of elements in the array
    const GLint size;

    virtual Value* clone() const = 0;
    virtual ~Value() {}

    // If type is one of GL_SAMPLER_*, the getTextures should provide a pointer to
    // the array of shared_ptr<Texture> stored by the uniform. And apply should
    // use the boundTexUnits argument as the argument for glUniform*.
    //
    // Otherwise, boundTexUnit should be ignored and whatever values contained in
    // the Value instance should be set to given location.
    //
    // `count' specifies how many actural uniforms are specified by the shader, and
    // should be used as input parameter to glUniform*
    virtual void apply(GLint location, GLsizei count, const GLint *boundTexUnits) const = 0;
    virtual const std::shared_ptr<Texture> * getTextures() const { return NULL; };

protected:
    Value(GLenum aType, GLint aSize) : type(aType), size(aSize) {}
  };

  template<typename T, int n>
  class CvecsValue : public Value {
    std::vector<glm::vec<n, T, glm::defaultp> > vs_;    // std::vector<Cvec<T, n> > vs_;

public:
    CvecsValue(const glm::vec<n, T, glm::defaultp> *vs, int size)
      : Value(_helper::getTypeForCvec<T,n>(), size), vs_(vs, vs + size) {
      assert(size > 0);
    }

    // construct from Cvecs of another type
    template<typename S>
    CvecsValue(const glm::vec<n, S, glm::defaultp>  *vs, int size)
      : Value(_helper::getTypeForCvec<T,n>(), size), vs_(size) {
      assert(size > 0);
      for (int i = 0; i < size; ++i) {
        for (int d = 0; d < n; ++d) {
          vs_[i][d] = T(vs[i][d]);
        }
      }
    }

    virtual Value* clone() const {
      return new CvecsValue(*this);
    }

    virtual void apply(GLint location, GLsizei count, const GLint *boundTexUnit) const {
      assert(count <= size);
      _helper::genericGlUniformv(location, count, &vs_[0]);
    }
  };

  class Matrix4sValue : public Value {
    // to pass the data into glUniformMatrix4fv, 
    // we need to have the internal buffer to be typed float.
    std::vector<glm::mat4> ms_;
public:
    Matrix4sValue(const glm::mat4 *m, int size)
      : Value(GL_FLOAT_MAT4, size), ms_(size) {
      assert(size > 0);
      for (int i = 0; i < size; ++i) {
        ms_[i] = m[i];    // m[i].writeToColumnMajorMatrix(&ms_[i][0]);
      }
    }

    virtual Value* clone() const {
      return new Matrix4sValue(*this);
    }

    virtual void apply(GLint location, GLsizei count, const GLint *boundTexUnit) const {
      assert(count <= size);
      _helper::genericGlUniformMatrix4v(location, count, &ms_[0]);
    }
  };

  class TexturesValue : public Value {
    std::vector<std::shared_ptr<Texture> > texs_;
public:
    TexturesValue(const std::shared_ptr<Texture> *tex, int size)
      : Value(tex[0]->getSamplerType(), size), texs_(tex, tex + size) {
      assert(size > 0);
      for (int i = 0; i < size; ++i) {
        assert(tex[i]->getSamplerType() == type);
      }
    }

    virtual Value* clone() const {
      return new TexturesValue(*this);
    }

    virtual void apply(GLint location, GLsizei count, const GLint *boundTexUnits) const {
      assert(count <= size);
      _helper::genericGlUniformv(location, count, boundTexUnits);
    }

    virtual const std::shared_ptr<Texture> *getTextures() const {
      return &texs_[0];
    }
  };
};

