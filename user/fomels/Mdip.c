/* 3-D dip estimation by plane wave destruction. */
/*
  Copyright (C) 2004 University of Texas at Austin
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <rsf.h>

#include "dip3.h"
#include "mask6.h"

int main (int argc, char *argv[])
{
    int n123, niter, order, nj1,nj2, i,j, liter, dim;
    int n[SF_MAX_DIM], rect[3], n4, nr, ir; 
    float p0, q0, *u, *p, pmin, pmax, qmin, qmax;
    char key[4];
    bool verb, *m1, *m2;
    sf_file in, out, mask, idip0, xdip0;

    sf_init(argc,argv);
    in = sf_input ("in");
    out = sf_output ("out");

    if (SF_FLOAT != sf_gettype(in)) sf_error("Need float type");

    dim = sf_filedims(in,n);
    if (dim < 2) n[1]=1;
    if (dim < 3) n[2]=1;
    n123 = n[0]*n[1]*n[2];
    nr = 1;
    for (j=3; j < dim; j++) {
	nr *= n[j];
    }

    if (1 == n[2]) {
	n4=0; 
    } else {
	if (!sf_getint("n4",&n4)) n4=2;
	/* what to compute in 3-D. 0: in-line, 1: cross-line, 2: both */ 
	if (n4 > 2) n4=2;
	if (2==n4) {
	    sf_putint(out,"n4",n4);
	    for (j=3; j < dim; j++) {
		snprintf(key,4,"n%d",j+2);
		sf_putint(in,key,n[j]);
	    }
	}
    }

    if (!sf_getint("niter",&niter)) niter=5;
    /* number of iterations */
    if (!sf_getint("liter",&liter)) liter=20;
    /* number of linear iterations */

    if (!sf_getint("rect1",&rect[0])) rect[0]=1;
    if (!sf_getint("rect2",&rect[1])) rect[1]=1;
    if (!sf_getint("rect3",&rect[2])) rect[2]=1;
    /* dip smoothness */

    if (!sf_getfloat("p0",&p0)) p0=0.;
    /* initial in-line dip */
    if (!sf_getfloat("q0",&q0)) q0=0.;
    /* initial cross-line dip */

    if (!sf_getint("order",&order)) order=1;
    /* [1,2,3] accuracy order */
    if (order < 1 || order > 3) 
	sf_error ("Unsupported order=%d, choose between 1 and 3",order);
    if (!sf_getint("nj1",&nj1)) nj1=1;
    /* in-line antialiasing */
    if (!sf_getint("nj2",&nj2)) nj2=1;
    /* cross-line antialiasing */

    if (!sf_getbool("verb",&verb)) verb = false;
    /* verbosity flag */
    if (!sf_getfloat("pmin",&pmin)) pmin = -FLT_MAX;
    /* minimum inline dip */
    if (!sf_getfloat("pmax",&pmax)) pmax = +FLT_MAX;
    /* maximum inline dip */
    if (!sf_getfloat("qmin",&qmin)) qmin = -FLT_MAX;
    /* minimum cross-line dip */
    if (!sf_getfloat("qmax",&qmax)) qmax = +FLT_MAX;
    /* maximum cross-line dip */

    /* initialize dip estimation */
    dip3_init(n[0], n[1], n[2], rect, liter);

    u = sf_floatalloc(n123);
    p = sf_floatalloc(n123);

    if (NULL != sf_getstring("mask")) {
	m1 = sf_boolalloc(n123);
	m2 = sf_boolalloc(n123);
	mask = sf_input("mask");
    } else {
	m1 = NULL;
	m2 = NULL;
	mask = NULL;
    }

    if (NULL != sf_getstring("idip")) {
	/* initial in-line dip */
	idip0 = sf_input("idip");
    } else {
	idip0 = NULL;
    }

    if (NULL != sf_getstring("xdip")) {
	/* initial cross-line dip */
	xdip0 = sf_input("xdip");
    } else {
	xdip0 = NULL;
    }

    for (ir=0; ir < nr; ir++) {
    	if (NULL != mask) {
	    sf_floatread(u,n123,mask);
	    mask32 (order, nj1, nj2, n[0], n[1], n[2], u, m1, m2);
	}

	/* read data */
	sf_floatread(u,n123,in);
	
	if (1 != n4) {
	    /* initialize t-x dip */
	    if (NULL != idip0) {
		sf_floatread(p,n123,idip0);
	    } else {
		for(i=0; i < n123; i++) {
		    p[i] = p0;
		}
	    }
	    
	    /* estimate t-x dip */
	    dip3(1, niter, order, nj1, verb, u, p, m1, pmin, pmax);
	    
	    /* write t-x dip */
	    sf_floatwrite(p,n123,out);
	}

	if (0 == n4) continue; /* done if only t-x dip */

	/* initialize t-y dip */
	if (NULL != xdip0) {
	    sf_floatread(p,n123,idip0);
	} else {
	    for(i=0; i < n123; i++) {
		p[i] = q0;
	    }
	}	

	/* estimate t-y dip */
	dip3(2, niter, order, nj2, verb, u, p, m2, pmin, pmax);
	
	/* write t-y dip */
	sf_floatwrite(p,n123,out);
    }
    
    exit (0);
}

/* 	$Id$	 */
