#pragma once
#include <iostream>
#include <fstream>
#include "ray.h"
#include "hitable.h"
#include "drand48.h"

//#define random(rAlbedo, b)   (rand()%(b - rAlbedo +1 ) +rAlbedo )
//#define drand48()	(float((rand()%100)) /100.0f)





vec3   reflect(const vec3& v, const vec3&n)
{
	return v - 2 * dot(v, n) * n;
}

vec3 randomPointInUintsphere()
{
	vec3  point;
	do
	{
		point = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
	} while (dot(point, point) >= 1.0);
	return point;
}


class Material
{
public:
	virtual bool scatter(const Ray& rayIn, const Hit_record& rec, vec3& attenuation, Ray& scattered) const = 0;


private:

};

class lambertian :public Material
{
private:
	vec3   albedo;
public:
	lambertian(const vec3& rAlbedo) : albedo(rAlbedo) {};
	virtual bool scatter(const Ray& rayIn, const Hit_record& rec, vec3& attenuation, Ray& scattered) const override
	{
		vec3 target = rec.normal + randomPointInUintsphere();
		scattered = Ray(rec.p, target);
		attenuation = albedo;
		return true;
	}

};


class metal : public Material
{
private:
	vec3	albedo;
	float   fuzz;
public:
	metal(const vec3& rAlbedo, const float f) : albedo(rAlbedo) 
	{
		if (f < 1)
			fuzz = f;
		else
			fuzz = 1.0f;
		std::cout << fuzz << std::endl;
	};
	virtual bool scatter(const Ray& rayIn, const Hit_record& rec, vec3& attenuation, Ray& scattered) const override
	{

		vec3	reflected = reflect(makeUnitVector(rayIn.direction()), rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * randomPointInUintsphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}
};




//this code for dielectrics material
// 反射系数的求解是是一个非常复杂的过程，Christophe Schlick这个人提供一个逼近公式，
// 这个公式被称为“ChristopheSchlick’s Approximation”。Wiki链接：
// https://en.wikipedia.org/wiki/Schlick%27s_approximation

float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}


bool refract(const vec3& rayIn, const vec3& normal, float rayRatio, vec3& refracted )
{
	vec3 unitRay = makeUnitVector(rayIn);
	float dt = dot(unitRay, normal);
	//  discrimmant is equal to (1- sin2平方)的平方；
	float discrimant = 1.0 - rayRatio * rayRatio *(1 -dt * dt);
	if(discrimant  > 0) 
	{
		//折射光线
		refracted = rayRatio*(unitRay - normal * dt) - normal*sqrt(discrimant);
		return true;
	}
	else
		return false;
}


class dielectric :public Material
{
private:
	float refractIndex;
	public:
	dielectric(float refIdx = 1.0f): refractIndex(refIdx) {};
	virtual bool scatter(const Ray& rayIn, const Hit_record& rec, vec3& attenuation, Ray& scattered) const override
	{
		vec3	outWardNormal;
		vec3	reflected = reflect(rayIn.direction(), rec.normal);
		float	rayRatio;
		attenuation = vec3(1.0, 1.0, 1.0);
		vec3	refracted;
		float reflectProb ;
		float cosine;
		if (dot( rayIn.direction(), rec.normal ) > 0)
		{
			outWardNormal = -rec.normal;
			rayRatio = refractIndex;
			cosine = dot(rayIn.direction(), rec.normal) / rayIn.direction().length();
			cosine = sqrt(1 - refractIndex * refractIndex * (1 - cosine * cosine));
		}
		else
		{
			outWardNormal = rec.normal;
			rayRatio = 1.0 / refractIndex;
			cosine = -dot(rayIn.direction(), rec.normal) / rayIn.direction().length();
		}

		if(refract( rayIn.direction(), outWardNormal, rayRatio, refracted))
		{
			reflectProb = schlick(cosine, refractIndex);
			 scattered = Ray(rec.p, refracted);
		}
		else
		{
			reflectProb = 1.0;
			 scattered = Ray(rec.p, reflected);
		}
			
		//if ( drand48() < reflectProb )
		//{
		//	scattered = Ray(rec.p, reflected);
		//}
		//else
		//{
		//	scattered = Ray(rec.p, refracted);
		//}
		return true;
	}

};
