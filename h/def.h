/*
 * def.h
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#ifndef DEF_H_
#define DEF_H_

//konstante
#define NEW      0
#define READY    1
#define BLOCKED  2
#define FINISHED 3

//lokovi
#define lock asm pushf; asm cli
#define unlock asm popf


#endif /* DEF_H_ */
