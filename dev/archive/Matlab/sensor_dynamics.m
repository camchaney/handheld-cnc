%% Sensor Rigid Body Dynamics
clear; clc;

syms v0x v0y v1x v1y v2x v2y v3x v3y lx ly wb th real

V0 = [v0x v0y 0]';
V1 = [v1x v1y 0]';
V2 = [v2x v2y 0]';
V3 = [v3x v3y 0]';

X0 = [-lx ly 0]'/2;
X1 = [lx ly 0]'/2;
X2 = [-lx -ly 0]'/2;
X3 = [lx -ly 0]'/2;

w = [0 0 wb]';

% Angular velocity calculations
W(:,1) = V0 + cross(w, X1 - X0);
W(:,2) = V0 + cross(w, X2 - X0);
W(:,3) = V0 + cross(w, X3 - X0);
W(:,4) = V1 + cross(w, X2 - X1);
W(:,5) = V1 + cross(w, X3 - X1);
W(:,6) = V2 + cross(w, X3 - X2);
W = W';

syms E1 E2 e1 e2 th real
E1 = cos(th)*[e1;0;0] + sin(th)*[0;e2;0];
E2 = -sin(th)*[e1;0;0] + cos(th)*[0;e2;0];

V0E = v0x*E1 + v0y*E2;
V1E = v1x*E1 + v1y*E2;
V2E = v2x*E1 + v2y*E2;
V3E = v3x*E1 + v3y*E2;

X0E = (-lx*E1 + ly*E2)/2;
X1E = (lx*E1 + ly*E2)/2;
X2E = (-lx*E1 - ly*E2)/2;
X3E = (lx*E1 - ly*E2)/2;

% Body velocity calculations
V(:,1) = V0 + cross(w, -X0);
V(:,2) = V1 + cross(w, -X1);
V(:,3) = V2 + cross(w, -X2);
V(:,4) = V3 + cross(w, -X3);

VE(:,1) = V0E + cross(w, -X0E);
VE(:,2) = V1E + cross(w, -X1E);
VE(:,3) = V2E + cross(w, -X2E);
VE(:,4) = V3E + cross(w, -X3E);