// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/MOLMEC/AMBER/amberTorsion.h>
#include <BALL/MOLMEC/AMBER/amber.h>
#include <BALL/MOLMEC/COMMON/forceFieldComponent.h>
#include <BALL/MOLMEC/COMMON/forceField.h>
#include <BALL/KERNEL/atom.h>
#include <BALL/KERNEL/bond.h>

#include <algorithm>

using namespace std;

namespace BALL 
{

	// default constructor
	AmberTorsion::AmberTorsion()
		:	ForceFieldComponent()
	{	
		// set component name
		setName("Amber Torsion");
	}


	// constructor
	AmberTorsion::AmberTorsion(ForceField& force_field)
		:	ForceFieldComponent(force_field)
	{
		// set component name
		setName( "Amber Torsion" );
	}


	// copy constructor
	AmberTorsion::AmberTorsion(const AmberTorsion& component)
		:	ForceFieldComponent(component)
	{
		// assign the torsion array
		torsion_ = component.torsion_;
	}

	// destructor
	AmberTorsion::~AmberTorsion()
	{
		// clear the torsion array
		torsion_.clear();
	}


	// setup the internal datastructures for the component
	bool AmberTorsion::setup()
	{
		if (getForceField() == 0) 
		{
			Log.error() << "AmberTorsion::setup: component not bound to force field" << endl;
			return false;
		}

		// clear torsion array
		torsion_.clear();

 		Options& options = getForceField()->options;
		if (options.has(AMBER_TORSIONS_ENABLED))
		{
			if (!options.getBool(AMBER_TORSIONS_ENABLED))
			{
				setEnabled(false);
				return true;
			}
			else
			{
				setEnabled(true);
			}
		}

		// extract the torsion parameters from the parameter file
		bool result;
		AmberFF* amber_force_field = dynamic_cast<AmberFF*>(force_field_);
		bool has_initialized_parameters = true;
		if ((amber_force_field == 0) || !amber_force_field->hasInitializedParameters())
		{
			has_initialized_parameters = false;
		}

		if (!has_initialized_parameters)
		{
			result = torsion_parameters_.extractSection(getForceField()->getParameters(), "Torsions");

			if (!result) 
			{
				Log.error() << "AmberTorsion::setup: cannot find section Torsions" << endl;
				return false;
			}
		}

		// calculate the torsions
		vector<Atom*>::const_iterator atom_it = getForceField()->getAtoms().begin();
		Atom::BondIterator it1;
		Atom::BondIterator it2;
		Atom::BondIterator it3;

		Atom*	a1;
		Atom*	a2;
		Atom*	a3;
		Atom*	a4;

		bool use_selection = getForceField()->getUseSelection();

		// proper torsion will be added to the torsion vector
		for (; atom_it != getForceField()->getAtoms().end(); ++atom_it) 
		{
			for (it1 = (*atom_it)->beginBond(); +it1 ; ++ it1) 
			{
				if (it1->getType() == Bond::TYPE__HYDROGEN) continue; // ignore H -bonds
				if (*atom_it == it1->getFirstAtom()) 
				{
					// central atoms
					a2 = *atom_it;
					a3 = const_cast<Atom*>(it1->getSecondAtom());

					for (it2 = (*atom_it)->beginBond(); +it2 ; ++it2) 
					{
						if (it2->getType() == Bond::TYPE__HYDROGEN) continue; // ignore H -bonds
						if (it2->getSecondAtom() != it1->getSecondAtom()) 
						{
							// determine the first atom
							if (it2->getFirstAtom() == *atom_it) 
							{
								a1 = const_cast<Atom*>(it2->getSecondAtom());
							} 
							else 
							{
								a1 = const_cast<Atom*>(it2->getFirstAtom());
							}
 
							for (it3 = const_cast<Atom*>(it1->getSecondAtom())->beginBond(); +it3 ; ++it3) 
							{
								if (it3->getType() == Bond::TYPE__HYDROGEN) continue; // ignore H -bonds
								if (it3->getFirstAtom() != a2 ) 
								{
									// determine the fourth atom a4
									if (it3->getFirstAtom() == a3)
									{
										a4 = const_cast<Atom*>(it3->getSecondAtom());
									} 
									else 
									{
										a4 = const_cast<Atom*>(it3->getFirstAtom());
									}

									if (!use_selection || (use_selection
									    && a1->isSelected()
									    && a2->isSelected()
									    && a3->isSelected()
									    && a4->isSelected()))
									{
										// search torsion parameters for (a1,a2,a3,a4)
										Atom::Type type_a1 = a1->getType();
										Atom::Type type_a2 = a2->getType();
										Atom::Type type_a3 = a3->getType();
										Atom::Type type_a4 = a4->getType();
										CosineTorsion::Values values;
									 	CosineTorsion::SingleData tmp;
										tmp.atom1 = a1;
										tmp.atom2 = a2;
										tmp.atom3 = a3;
										tmp.atom4 = a4;

										bool found = false;

										if (torsion_parameters_.assignParameters(values, type_a1, type_a2, type_a3, type_a4)) 
										{
											found = true;
										} 
										else if (torsion_parameters_.assignParameters(values, Atom::ANY_TYPE, type_a2, type_a3, Atom::ANY_TYPE)) 
										{
											found = true;
										}
										if (found) 
										{
											for (unsigned char j = 0; j < values.n; j++) 
											{
												tmp.values = values.values[j];
												torsion_.push_back(tmp);
											}
										} 
										else 
										{
											getForceField()->error() << "AmberTorsion::setup: cannot find torsion parameter for:"
												<< force_field_->getParameters().getAtomTypes().getTypeName(type_a1) << "-"
												<< force_field_->getParameters().getAtomTypes().getTypeName(type_a2) << "-"
												<< force_field_->getParameters().getAtomTypes().getTypeName(type_a3) << "-"
												<< force_field_->getParameters().getAtomTypes().getTypeName(type_a4) 
												<< " (atoms are: " << a1->getFullName() << "/" << a2->getFullName() 
												<< "/" << a3->getFullName() << "/" << a4->getFullName() << ")" << endl;

											getForceField()->getUnassignedAtoms().insert(a1);
											getForceField()->getUnassignedAtoms().insert(a2);
											getForceField()->getUnassignedAtoms().insert(a3);
											getForceField()->getUnassignedAtoms().insert(a4);
										}
									}
								} 
							}
						}
					}
				}
			}
		}

		if (!has_initialized_parameters)
		{
			result = improper_parameters_.extractSection(getForceField()->getParameters(), "ImproperTorsions");

			if (!result) 
			{
				Log.error() << "AmberTorsion::setup: cannot find section ImproperTorsions" << endl;
				return false;
			}
		}
	

		// Improper torsions will be added to the torsion array
		atom_it = getForceField()->getAtoms().begin();

		// find all improper torsion atoms: their names are stored in 
		// the section ResidueImproperTorsions
		if (!has_initialized_parameters)
		{
			result = impropers_.extractSection(getForceField()->getParameters(), "ResidueImproperTorsions");
			if (!result)
			{
				Log.error() << "cannot find section ResidueImproperTorsions" << endl;
				return false;
			}
		}
		
		// check for each potential improper torsion atom (every atom having three bonds)
		// whether it is contained in the list of impropers
		for ( ; atom_it != getForceField()->getAtoms().end(); ++atom_it) 
		{
			if ((*atom_it)->countBonds() == 3)
			{
				String res_name;
				int terminal = 0;
				Residue* res = (*atom_it)->getAncestor(RTTI::getDefault<Residue>());
				if (res == 0)
				{
					if ((*atom_it)->getFragment() != 0)
					{
						res_name = (*atom_it)->getFragment()->getName();
						res_name.trim();
					}
				} 
				else 
				{
					res_name = res->getName();
					res_name.trim();

					String suffix = "-";
					if (res->isNTerminal())
					{
						suffix = "-N";
					}
					if (res->isCTerminal())
					{
						suffix = "-C";
					}
					if (res->hasProperty(Residue::PROPERTY__HAS_SSBOND))
					{
						suffix += "S";
					}
					if (suffix != "-")
					{
						res_name += suffix;
					}
				}

				String key;
				if (res_name != "")
				{
					key = res_name + " " + (*atom_it)->getName();
					switch (terminal)
					{
						case 1:		key += " N"; break;
						case 2:		key += " C"; break;
						default:	key += " -";
					}
				}

				if (impropers_.has(key))
				{
					for (it1 = (*atom_it)->beginBond(); +it1 ; ++ it1) 
					{
						a3 = *atom_it;
						a4 = it1->getPartner(*a3);

						for (it2 = it1, ++it2; +it2 ; it2++) 
						{
							a2 = it2->getPartner(**atom_it);

							for (it3 = it2, ++it3; +it3 ; it3++) 
							{
								a1 = it3->getPartner(**atom_it);

								// test if (a1,a2,a3,a4) is improper torsion and search torsion parameters for (a1,a2,a3,a4)
								CosineTorsion::SingleData tmp;
								CosineTorsion::Values values;

								tmp.atom1 = a1;
								tmp.atom2 = a2;
								tmp.atom3 = a3;
								tmp.atom4 = a4;

								// sort IJL according to the lexicographic order
								// of their type name (AMBER!!)
								if (a1->getTypeName() > a2->getTypeName()) swap(a1, a2);
								if (a1->getTypeName() > a4->getTypeName()) swap(a1, a4);
								if (a2->getTypeName() > a4->getTypeName()) swap(a2, a4);

								if (!use_selection || (use_selection
								    && a1->isSelected()
								    && a2->isSelected()
								    && a3->isSelected()
								    && a4->isSelected()))
								{
									Atom::Type type_a1 = a1->getType();
									Atom::Type type_a2 = a2->getType();
									Atom::Type type_a3 = a3->getType();
									Atom::Type type_a4 = a4->getType();

									bool found = false;

									if (improper_parameters_.hasParameters(type_a1, type_a2, type_a3, type_a4)) 
									{
										improper_parameters_.assignParameters(values, type_a1, type_a2, type_a3, type_a4);
										found = true;
									} 
									else if (improper_parameters_.hasParameters(Atom::ANY_TYPE, type_a2, type_a3, type_a4))
									{
										improper_parameters_.assignParameters(values, Atom::ANY_TYPE, type_a2, type_a3, type_a4);
										found = true;
									} 
									else if (improper_parameters_.hasParameters(Atom::ANY_TYPE, Atom::ANY_TYPE, type_a3, type_a4)) 
									{
										improper_parameters_.assignParameters(values, Atom::ANY_TYPE, Atom::ANY_TYPE, type_a3, type_a4);
										found = true;
									} 
									else if (improper_parameters_.hasParameters(Atom::ANY_TYPE, type_a2, type_a3, Atom::ANY_TYPE)) 
									{
										improper_parameters_.assignParameters(values, Atom::ANY_TYPE, type_a2, type_a3, Atom::ANY_TYPE);
										found = true;
									}

									if (found)	
									{
										for (Position j = 0; j < values.n; j++) 
										{
											tmp.values = values.values[j];
											torsion_.push_back(tmp);
										}
									}
								}
							}
						}	
					}
				}
			}
		}

		return true;
	}

	// calculates the current energy of this component
	double AmberTorsion::updateEnergy() 
	{
		double cosphi;

		Vector3	a21;
		Vector3 a23;
		Vector3 a34;
		Vector3 cross2321;
		Vector3 cross2334;

		energy_ = 0;

		vector<SingleAmberTorsion>::const_iterator it = torsion_.begin(); 

		bool use_selection = getForceField()->getUseSelection();

		for (; it != torsion_.end(); it++) 
		{
			const Atom* atom1 = it->atom1;
			const Atom* atom2 = it->atom2;
			const Atom* atom3 = it->atom3;
			const Atom* atom4 = it->atom4;

			if (!use_selection || (use_selection &&
			    (   atom1->isSelected() || atom2->isSelected()
			     || atom3->isSelected() || atom4->isSelected())))
			{
				a21 = atom1->getPosition() - atom2->getPosition();
				a23 = atom3->getPosition() - atom2->getPosition();
				a34 = atom4->getPosition() - atom3->getPosition();

				cross2321 = a23 % a21;
				cross2334 = a23 % a34;

				double length_cross2321 = cross2321.getLength();
				double length_cross2334 = cross2334.getLength();

				if (length_cross2321 != 0 && length_cross2334 != 0) 
				{
					cross2321 /= length_cross2321;
					cross2334 /= length_cross2334;

					cosphi = cross2321 * cross2334;

					if (cosphi > 1.0)
					{
						cosphi = 1.0;
					}
					if (cosphi < -1.0)
					{
						cosphi = -1.0;
					}

					energy_ += it->V * (1 + cos(it->f * acos(cosphi) - it->phase));
				}
			}
		}

		return energy_;
	}

	// calculates and adds its forces to the current forces of the force field
	void AmberTorsion::updateForces()
	{
		double cosphi;
		double dEdphi;

		Vector3	ab;	// vector from atom2 to atom1
		Vector3 cb;	// vector from atom2 to atom3
		Vector3 dc;	// vector from atom3 to atom4

		bool use_selection = getForceField()->getUseSelection();

		vector<SingleAmberTorsion>::iterator it = torsion_.begin(); 

		for ( ; it != torsion_.end(); it++) 
		{
			Atom* atom1 = it->atom1;
			Atom* atom2 = it->atom2;
			Atom* atom3 = it->atom3;
			Atom* atom4 = it->atom4;

			if (!use_selection || (use_selection &&
			    (   atom1->isSelected() || atom2->isSelected()
			     || atom3->isSelected() || atom4->isSelected())))
			{
				ab = atom1->getPosition() - atom2->getPosition();
				double length_ab = ab.getLength();
				Vector3 ba = atom2->getPosition() - atom1->getPosition();
				cb = atom3->getPosition() - atom2->getPosition();
				double length_cb = cb.getLength();
				dc = atom4->getPosition() - atom3->getPosition();
				double length_dc = dc.getLength();

				if (length_ab != 0 && length_cb != 0 && length_dc != 0) 
				{
					Vector3  t = ba % cb;   // cross product of cb and ba
					Vector3  u = cb % dc;   // cross product of cb and dc

					double length_t2 = t.getSquareLength();
					double length_u2 = u.getSquareLength();

					double length_t = sqrt(length_t2);
					double length_u = sqrt(length_u2);

					if (length_t != 0 && length_u != 0) 
					{
						cosphi = (t * u) / (length_t * length_u);

						if (cosphi > 1.0)
						{
							cosphi = 1.0;
						}
						if (cosphi < -1.0)
						{
							cosphi = -1.0;
						}

						// multiply with the barrier height and a factor
						// for unit conversion: 1e13: kJ/(mol A) -> J/(mol m)
						//  AVOGADRO: J/mol -> J
						dEdphi = (-it->V) * (1e13 / Constants::AVOGADRO) * it->f * sin(it->f * acos(cosphi) - it->phase);

						double direction = (t % u) * cb;
						if (direction > 0.0)
						{
							dEdphi = -dEdphi;
						}

						Vector3 ca = atom3->getPosition() - atom1->getPosition();
						Vector3 db = atom4->getPosition() - atom2->getPosition();
						Vector3 dEdt =   (float)(dEdphi / (length_t2 * cb.getLength())) * (t % cb);
						Vector3 dEdu = - (float)(dEdphi / (length_u2 * cb.getLength())) * (u % cb);
	

						if (!use_selection)
						{
							atom1->getForce() += dEdt % cb;
							atom2->getForce() += ca % dEdt + dEdu % dc;
							atom3->getForce() += dEdt % ba + db % dEdu;
							atom4->getForce() += dEdu % cb;
						}
						else
						{
							if (atom1->isSelected()) atom1->getForce() += dEdt % cb;
							if (atom2->isSelected()) atom2->getForce() += ca % dEdt + dEdu % dc;
							if (atom3->isSelected()) atom3->getForce() += dEdt % ba + db % dEdu;
							if (atom4->isSelected()) atom4->getForce() += dEdu % cb;
						}
					}
				}
			}
		}
	}

} // namespace BALL
