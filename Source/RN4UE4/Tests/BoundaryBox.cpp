// Fill out your copyright notice in the Description page of Project Settings.

#include "BoundaryBox.h"

ABoundaryBox::ABoundaryBox()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
}

