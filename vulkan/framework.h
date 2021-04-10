#pragma once

#include "../submodule/app-service-sandwich/AppServiceSandwich/Assertions.h"

#define DECLARE_MOVEABLE_TYPE(T) \
private: \
	void move_members(T&&); \
	void free_members(); \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t){ move_members(std::move(t)); }; \
	T& operator =(T&& t){ free_members(); move_members(std::move(t)); return *this; }; \
	~T() { free_members(); }; 

#define DECLARE_MOVEABLE_COPYABLE_TYPE(T) \
private: \
	void copy_members(const T&); \
	void move_members(T&&); \
	void free_members(); \
public: \
	T(const T& t) { copy_members(t); } \
	T& operator =(const T& t) { free_members(); copy_members(t); return *this; } \
	T(T&& t){ move_members(std::move(t)); }; \
	T& operator =(T&& t){ free_members(); move_members(std::move(t)); return *this; }\
	~T() { free_members(); }

#define DECLARE_DEFAULT_MOVEABLE_TYPE(T) \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t) = default; \
	T& operator =(T&& t) = default; 

#define DECLARE_DEFAULT_MOVEABLE_COPYABLE_TYPE(T) \
public: \
	T(const T&) = default; \
	T& operator =(const T&) = default; \
	T(T&& t) = default; \
	T& operator =(T&& t) = default; 

#define DECLARE_NOT_MOVEABLE_OR_COPYABLE_TYPE(T) \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t) = delete; \
	T& operator =(T&& t) = delete; 