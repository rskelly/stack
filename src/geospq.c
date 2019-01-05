// https://gis.stackexchange.com/questions/307621/postgis-geometry-to-geos-geometry-without-lwgeom-dependency
// Build with: gcc -I/usr/include/postgresql src/geospq.c -lpq -lgeos_c -o bin/geospq

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <libpq-fe.h>
#include <geos_c.h>

void notice(const char* m, ...) {
	va_list lst;
	va_start(lst, m);
	const char* s = va_arg(lst, const char*);
	printf(m, s);
	printf("\n");
}

int main(int argc, char** argv) {
	
	PGconn* conn = PQconnectdb("host=localhost dbname=test user=rob");
	if(!conn) {
		printf("Failed to connect.\n");
		return 1;
	}

	PGresult* res = PQexec(conn, "SELECT encode(ST_AsEWKB(geom), 'hex') FROM test LIMIT 1");
	if(!res) {
		printf("No result returned.\n");
		return 1;
	}

	const unsigned char* geom = PQgetvalue(res, 0, 0);
	int len = strlen(geom);

	PQfinish(conn);

	printf("Binary string and length: %s %d\n", geom, len);

	initGEOS(&notice, &notice);

	GEOSWKBReader* reader = GEOSWKBReader_create();
	if(!reader) {
		printf("Failed to create reader.\n");
		return 1;
	}

	GEOSGeometry* g = GEOSWKBReader_readHEX(reader, geom, len);
	if(!g) {
		printf("Failed to parse binary string.\n");
		return 1;
	}
	
	GEOSWKBReader_destroy(reader);

	const GEOSGeometry* r = GEOSGetExteriorRing(g);
	if(!r) {
		printf("Failed to retrieve exterior ring.\n");
		return 1;
	}

	const GEOSCoordSequence* s = GEOSGeom_getCoordSeq(r);
	if(!s) {
		printf("Failed to retrieve coord sequence.\n");
		return 1;
	}

	int n;
	double x, y;
	n = GEOSGeomGetNumPoints(r);//4;//GEOSCoordSeq_getSize(s, &n);
	printf("Number of points: %d\n", n);
	for(int i = 0; i < n; ++i) {
		GEOSCoordSeq_getX(s, i, &x);
		GEOSCoordSeq_getY(s, i, &y);
		printf("Coord %d: %f %f\n", i, x, y);
	}

	GEOSGeom_destroy(g);
	finishGEOS();
	
	return 0;
}