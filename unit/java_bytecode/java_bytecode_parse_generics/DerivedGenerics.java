public class DerivedGenerics {
    DerivedGenericInst new1;
    DerivedGenericInst2 new2;
    DerivedGenericUninst<Integer> new3;
    DerivedGenericMixed1<String> new4;
    DerivedGenericMixed2<String> new5;
    ExtendsNotGeneric new6;
    ContainsInnerClass new7;
    ContainsInnerClassGeneric<String> new8;
    ThreeHierarchy new9;
    ImplementsInterface new10;
    ImplementsInterfaceGenericSpecialised new11;
    ImplementsInterfaceGenericUnspec<String> new12;
    ImplementsMultipleInterfaces new13;
    ExtendsAndImplements new14;
    ExtendsAndImplementsGeneric new15;
}

class DerivedGenericInst extends Generic<Interface_Implementation>
{
    // This class is to test instantiating a non-generic subclass of a generic class
    // with the base class having only one type parameter.
}

class DerivedGenericInst2 extends
GenericTwoParam<Interface_Implementation,Integer>
{
    // This class is to test instantiating a non-generic subclass of a generic class
    // with the base class having two type parameters.
}

class DerivedGenericUninst<T> extends Generic<T>
{
    T newField;

    // This class is to test instantiating a generic subclass of a generic class
    // with the base class having only one parameter, but the type parameter is
    // not specialised.
}

class DerivedGenericMixed1<T> extends Generic<Interface_Implementation>
{
    T newField;

    // This class is to test instantiating a generic subclass of a generic class
    // with the base class having only one type parameter.
}

class DerivedGenericMixed2<T> extends GenericTwoParam<T, Integer>
{
    T newField;

    // This class is to test instantiating a generic subclass of a generic class
    // with the base class having two type parameters, where one is specialised
    // and the other is not.
}

class ContainsInnerClass {

    InnerClass ic;
    InnerClassGeneric<String> icg;

    // This class is to test inner classes that extend generic types.
    class InnerClass extends Generic<Integer> {
    }

    class InnerClassGeneric<T> extends Generic<T> {
    }
}

class ContainsInnerClassGeneric<T> {

    InnerClass ic;

    // This class is to test inner classes that extend generic types when the
    // outer class in generic.
    class InnerClass extends Generic<T> {
    }
}

class ThreeHierarchy extends DerivedGenericMixed2<String> {

    // This class extends a specialised class that extends another generic
    // class.

}

class ImplementsInterfaceGenericSpecialised implements InterfaceGeneric<Integer>
 {

    public Integer someMethod() {
        return 0;
    }

}

class ImplementsInterfaceGenericUnspec<E> implements InterfaceGeneric<E> {

    public E someMethod() {
        return null;
    }

}

class ImplementsMultipleInterfaces implements
InterfaceGeneric<Integer>, Interface
{

  public Integer someMethod() {
    return 0;
  }

  public int getX() {
    return 0;
  }
}

class ExtendsAndImplements extends Generic<Integer> implements Interface,
InterfaceGeneric<Integer>
{
  public Integer someMethod() {
    return 0;
  }

  public int getX() {
    return 0;
  }
}

class ExtendsAndImplementsGeneric<T> extends GenericTwoParam<T, Integer>
implements Interface,
InterfaceGeneric<T>
{
  T f;

  public T someMethod() {
    return f;
  }

  public int getX() {
    return 0;
  }
}
