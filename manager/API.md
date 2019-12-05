# Manager API

## -> /manager/add

```
d42, /drones/d42
```

## -> /manager/remove

```
d42
```

## -> /manager/drones/d42/stack

```
@CLEAR
@ZONE 0.3(m)
@SPEED 0.5(m/s)

// manual move
MOVE x=20 y=10 z=100 duration=5(s)
MOVE x=20 y=10 z=100 duration=5(s) limit=5(s)
MOVE x=20 y=10 z=100 speed=0.5(m/s) event=e137
MOVE x=20 y=10 z=100 acc=0.2(m/s^2) zone=0.1(m)

// generated move
MOVE x=20 y=10 z=100 delta=0.1(s)
MOVE x=20 y=10 z=101 delta=0.1(s)
MOVE x=20 y=10 z=102 delta=0.1(s)
MOVE x=20 y=10 z=103 delta=0.1(s)

// other
HOLD time=10(s)
CIRCLE
ARC
JMP -10
```

## <- /manager/drones/d42/event

```
e137
```

## <- /manager/drones/d42/state

```
x, y, z, X, Y, Z
x, y, z, X, Y, Z
```

## <- /manager/drones/d42/trajectories

```
x=20 y=10 z=100
x=20 y=10 z=100
x=20 y=10 z=100
x=20 y=10 z=100
```
