// Fill out your copyright notice in the Description page of Project Settings.

#include "BoundaryBox.h"
#include "RakNetRP.h"

ABoundaryBox::ABoundaryBox()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
}

